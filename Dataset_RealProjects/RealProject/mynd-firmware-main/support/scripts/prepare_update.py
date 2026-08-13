#!/usr/bin/env python3

import os
import sys
import argparse
import logging
import struct
import math
from random import randrange

import crcmod

from typing import List

# import pdb

DFU_PACKET_TYPE_INIT = 0
DFU_PACKET_TYPE_FW_HEADER = 1
DFU_PACKET_TYPE_DATA = 2

# Predefined component ids
COMPONENT_ID_MCU = 0
COMPONENT_ID_DSP = 1
COMPONENT_ID_BLUETOOTH = 2
COMPONENT_ID_HDMI = 3
COMPONENT_ID_SWATX = 4
COMPONENT_ID_SWARX = 5

COMPONENT_ID_MCU_BANK0 = 6 # Alternative MCU component ID for banks(slots)
COMPONENT_ID_MCU_BANK1 = 7 # Alternative MCU component ID for banks(slots)

COMPONENT_ID_DAB = 8

MAGIC = 0xBEEFCAFE

ENCODING_KEY = "TEUFELDEV"

ProjectIDs = [
              "OA2302",  # Mynd
              ]

crc32_func = crcmod.mkCrcFun(0x104c11db7, initCrc=0, xorOut=0xFFFFFFFF)


class FirmwareUpdater:

    __project_id = ""

    # Optional vars
    __reboot_after = True
    __force_update = False
    __encryption = True
    __chunk_size = 512
    __min_data_in_chunk = 256

    # tuple: (component_id: int, fw_path: str, fw_signature: bytearray)
    # TODO: that can be improved by using type alias
    __fw_opt = []

    def __init__(self, project_id: str, reboot_after: bool = True,
                 force_update: bool = False,
                 encryption: bool = True,
                 chunk_size: int = 512,
                 min_data_size: int = 256):
        """Init function

        Parameters:
            project_id: Project ID string in according to Teufel's projects format, e.g. AC1901
            reboot_after: Reboot after update
            force_update: Force update process even when the same firmware version already installed
            encryption: Use simple encryptions for chunks
            chunk_size: Split targer firmware into chunks with specified size
            min_data_size: Minimal data size in chunk, e.g. chunk size: 512 bytes, which includes header, data: 256 bytes
        """

        self.__project_id = project_id
        self.__reboot_after = reboot_after
        self.__force_update = force_update
        self.__encryption = encryption
        self.__chunk_size = chunk_size
        self.__min_data_in_chunk = min_data_size

    def __make_global_header(self) -> bytearray:

        p = struct.pack("<I", MAGIC)

        # Packet type - Init
        p += struct.pack("<B", DFU_PACKET_TYPE_INIT)

        # FW/Component number
        p += struct.pack("<B", len(self.__fw_opt))

        # Reserved 1 byte
        p += struct.pack("<B", 0)

        # Flags
        flags = int(0)

        if self.__project_id:
            flags |= (1 << 3)
        if self.__force_update:
            flags |= (1 << 2)
        if self.__encryption:
            # flags |= (0 << 4)
            # flags |= (1 << 5)
            flags |= 0x10

        p += struct.pack(">B", flags)

        # 8 bytes project id as ASCII representation
        bytes2 = bytearray([0, 0]) + \
            bytearray(self.__project_id, encoding='utf8')

        for _, b in enumerate(bytes2):
            p += struct.pack("<B", b)

        # Add padding bytes
        p += bytearray(self.__chunk_size - len(p))

        return p

    def __prepare_fw_header(self, component_id: int, chunks: int, size: int, crc: int, signature: bytearray) -> bytearray:

        p = struct.pack("<I", MAGIC)

        # Packet type - Init
        p += struct.pack("<B", DFU_PACKET_TYPE_FW_HEADER)

        # Component id
        p += struct.pack("<B", component_id)

        # Chunks
        p += struct.pack("<H", chunks)

        # Size of fw in bytes
        p += struct.pack("<I", size)

        # CRC32
        p += struct.pack("<I", crc)

        if len(signature) > 0:
            p += signature

        # Add padding bytes
        p += bytearray(self.__chunk_size - len(p))

        return p

    def __create_data_blob(self, component_id: int, chunk_num: int, data: bytearray) -> bytearray:

        p = struct.pack("<I", MAGIC)

        # Packet type - Data
        p += struct.pack("<B", DFU_PACKET_TYPE_DATA)

        # Component id
        p += struct.pack("<B", component_id)

        # Chunk number
        p += struct.pack("<H", chunk_num)

        p += struct.pack("<I", len(data))

        crc32 = crc32_func(data)
        # print(hex(crc32))

        # Checksum
        p += struct.pack("<I", crc32)

        # Add data, padding and return
        return p + data + bytearray(self.__chunk_size - len(p) - len(data))

    def __vineger_encode(self, data: bytearray, key: bytearray) -> bytearray:

        shifts = bytearray()
        res = bytearray()

        for _, c in enumerate(key):
            if not c.isalpha():
                print("ENCRYPTION, KEY IS NOT ALPHABETIC!")
                exit(1)

            if c.islower():
                shifts.append(ord(c) - 0x61)
            else:
                shifts.append(ord(c) - 0x41)

        j = 0
        for i, c in enumerate(data):
            # c is integer here because bytearray type
            if c > 0x7f:
                res.append(data[i])
                continue
            if not chr(c).isalpha():
                res.append(data[i])
                continue

            if chr(c).islower():
                res.append(((c + shifts[j] - 0x61 + 26) % 26) + 0x61)
            else:
                res.append(((c + shifts[j] - 0x41 + 26) % 26) + 0x41)

            j = (j + 1) % len(key)

        return res

    def __prepare_fw(self, component_id: int, fw_data: bytearray, signature: bytearray) -> bytearray:

        data = bytearray()

        # minimal batch of data that can be added to a chunk
        batches_per_chunk = math.floor(
            (self.__chunk_size - 32)/self.__min_data_in_chunk)

        # Real bytes of data which fit in one chunk
        bytes_per_chunk = batches_per_chunk*self.__min_data_in_chunk

        print("\nSummary:")
        print("  - chunk size: {} bytes".format(self.__chunk_size))
        print("  - batch size: {} bytes".format(self.__min_data_in_chunk))
        print("  - batches per chunk: {}".format(batches_per_chunk))
        print("  - bytes of data per chunk: {}".format(bytes_per_chunk))
        print("  - signature: ON")
        if self.__encryption:
            print("  - encryption: ON")
        else:
            print("  - encryption: OFF")

        print("\n")

        num_of_chunks = math.ceil(len(fw_data)/bytes_per_chunk)

        fw_header = self.__prepare_fw_header(
            component_id, num_of_chunks, len(fw_data), crc32_func(fw_data), signature)

        for i in range(len(fw_data)//bytes_per_chunk):
            if self.__encryption:
                data += self.__create_data_blob(component_id, i+1,
                                                self.__vineger_encode(fw_data[i*bytes_per_chunk:i*bytes_per_chunk+bytes_per_chunk], ENCODING_KEY))
            else:
                data += self.__create_data_blob(component_id, i+1,
                                                fw_data[i*bytes_per_chunk:i*bytes_per_chunk+bytes_per_chunk])

        # Last packet
        # Add the last blob only if there are leftovers data
        if len(fw_data) % bytes_per_chunk:
            if self.__encryption:
                data += self.__create_data_blob(component_id, len(fw_data)//bytes_per_chunk + 1,
                                                self.__vineger_encode(fw_data[-(len(fw_data) % bytes_per_chunk):], ENCODING_KEY))
            else:
                data += self.__create_data_blob(component_id, len(fw_data)//bytes_per_chunk + 1,
                                                fw_data[-(len(fw_data) % bytes_per_chunk):])

        return fw_header + data

    def add_firmware(self, component_id: int, fname: str, signature: str) -> None:

        with open(signature, "rb") as binary_file:
            sign = bytearray(binary_file.read())

        fw = bytearray()
        with open(fname, "rb") as binary_file:
            fw = bytearray(binary_file.read())

        self.__fw_opt.append((component_id, fname, sign))

    def create(self, fname: str) -> None:

        if len(self.__fw_opt) == 0:
            print("Error: first add firmwares")
            return

        print("prepare {}, with {} components".format(fname, len(self.__fw_opt)))

        init_packet = self.__make_global_header()

        fw_packets = bytearray()

        # __fw_opt is tuple: (component_id: int, fw_path: str, fw_signature: bytearray)
        for f in self.__fw_opt:
            with open(f[1], "rb") as binary_file:
                fw = bytearray(binary_file.read())

            fw_packets += self.__prepare_fw(f[0], fw, f[2])

        with open(fname, "wb") as f:
            f.write(init_packet + fw_packets)

    def shuffle(self, fname: str) -> None:
        """Shuffle update firmware"""
        fsize = os.path.getsize(fname)
        print("fsize - {}".format(fsize))
        print("chunks - {}".format(fsize/self.__chunk_size))

        # with open(fname, "wb") as f:
        #     f.seek(, 0)
        #     f.read(self.__chunk_size)

    def summarize(self) -> None:
        """Summaryse update firmware"""
        pass

def main(argv):
    parser = argparse.ArgumentParser(
        description="""Create update file for bootloader. Example: python3 ./prepare_update.py -p AC1902
                                                                        --mcu=mcu.bin
                                                                        --dsp=dsp.bin
                                                                        -k ./support/keys/teufel_dev_private.pem""")
    group = parser.add_mutually_exclusive_group()
    group.add_argument('-p', '--project-id',
                       help='Project ID', required=False, choices=ProjectIDs)
    group.add_argument('--verify-file',
                       help='Verify update file', required=False)
    parser.add_argument(
        '-i', '--input', help='Input file name', required=False)
    parser.add_argument(
        '-c', '--chunk-size', type=int, default=512, help='Chunk size', required=False)

    # foo_parser = argparse.ArgumentParser(parents=[parent_parser])
    parser.add_argument(
        '-o', '--output', type=str, default='update.bin', help='Output file name', required=False)
    parser.add_argument('--mcu', help='MCU firmware', required=False)
    parser.add_argument('--dsp', help='DSP firmware', required=False)
    parser.add_argument(
        '--bluetooth', help='Bluetooth firmware', required=False)
    parser.add_argument('--hdmi', help='HDMI firmware', required=False)
    parser.add_argument('--swatx', help='SWA TX firmware', required=False)
    parser.add_argument('--swarx', help='SWA RX firmware', required=False)
    parser.add_argument('--mcu-bank0', help='MCU firmware Bank0', required=False)
    parser.add_argument('--mcu-bank1', help='MCU firmware Bank1', required=False)
    parser.add_argument('--dab', help='DAB module', required=False)

    parser.add_argument('-r', '--reset', type=bool, default=True,
                        help='Reset target at the end', required=False)
    parser.add_argument('--no-verify', type=bool, default=False,
                        help='Practice exercise for testing', required=False)
    parser.add_argument('--force', type=bool, default=False,
                        help='Forcing update component even with the same CRC sum', required=False)
    parser.add_argument('--no-encryption', default=False,
                        help='Disable encryption', required=False, action='store_true')
    parser.add_argument('-k', '--pem-key',
                        help='Certificate for signature', required=True)

    parser.add_argument('--shuffle', default=False,
                        help='Shuffle update file (for testing non-sequential update)', required=False,
                        action='store_true')

    parser.add_argument('-v', '--verbose', default=0,
                        help='Verbose option', action='count')
    args = parser.parse_args()

    # Add randomisation to signature file, to prevent removing the file when running multiple threads.
    signature_file = "sign.sha1.{}".format(randrange(0, 100000, 1))

    # Logger level
    lev = logging.INFO
    if (args.verbose > 0):
        lev = logging.DEBUG
    logging.basicConfig(format='%(message)s', level=lev)

    if (args.verify_file):
        print("verify file!")
        exit(0)

    # do_prepare here ...

    upd = FirmwareUpdater(
        args.project_id, chunk_size=args.chunk_size, encryption=(not args.no_encryption))

    if args.mcu is not None:
        # Signature for mcu firmware
        # os.system(
        #     "openssl dgst -sha1 -sign ../support/keys/teufel_dev_private.pem -out ./sign.sha1 {}".format(args.mcu))

        os.system(
            "openssl dgst -sha1 -sign {} -out ./{} {}".format(args.pem_key, signature_file, args.mcu))

        # Add mcu firmware
        upd.add_firmware(COMPONENT_ID_MCU, args.mcu, "./{}".format(signature_file))

    if args.dsp is not None:
        # Signature for dsp firmware
        os.system(
            "openssl dgst -sha1 -sign {} -out ./{} {}".format(args.pem_key, signature_file, args.dsp))

        # Add dsp firmware
        upd.add_firmware(COMPONENT_ID_DSP, args.dsp, "./{}".format(signature_file))

    if args.bluetooth is not None:
        # Signature for bluetooth firmware
        os.system(
            "openssl dgst -sha1 -sign {} -out ./{} {}".format(args.pem_key, signature_file, args.bluetooth))

        # Add bluetooth firmware
        upd.add_firmware(COMPONENT_ID_BLUETOOTH, args.bluetooth, "./{}".format(signature_file))

    if args.hdmi is not None:
        # Signature for hdmi firmware
        os.system(
            "openssl dgst -sha1 -sign {} -out ./{} {}".format(args.pem_key, signature_file, args.hdmi))

        # Add hdmi firmware
        upd.add_firmware(COMPONENT_ID_HDMI, args.hdmi, "./{}".format(signature_file))

    if args.swatx is not None:
        # Signature for swatx firmware
        os.system(
            "openssl dgst -sha1 -sign {} -out ./{} {}".format(args.pem_key, signature_file, args.swatx))

        # Add swatx firmware
        upd.add_firmware(COMPONENT_ID_SWATX, args.swatx, "./{}".format(signature_file))

    if args.mcu_bank0 is not None:
        # Signature for mcu bank0 firmware
        os.system(
            "openssl dgst -sha1 -sign {} -out ./{} {}".format(args.pem_key, signature_file, args.mcu_bank0))

        # Add mcu bank0 firmware
        upd.add_firmware(COMPONENT_ID_MCU_BANK0, args.mcu_bank0, "./{}".format(signature_file))

    if args.mcu_bank1 is not None:
        # Signature for mcu bank1 firmware
        os.system(
            "openssl dgst -sha1 -sign {} -out ./{} {}".format(args.pem_key, signature_file, args.mcu_bank1))

        # Add mcu bank1 firmware
        upd.add_firmware(COMPONENT_ID_MCU_BANK1, args.mcu_bank1, "./{}".format(signature_file))

    if args.dab is not None:
        # Signature for dab firmware
        os.system(
            "openssl dgst -sha1 -sign {} -out ./{} {}".format(args.pem_key, signature_file, args.dab))

        # Add dab firmware
        upd.add_firmware(COMPONENT_ID_DAB, args.dab, "./{}".format(signature_file))

    upd.create(args.output)

    if args.shuffle:
        upd.shuffle(args.output)

    if os.path.exists("./{}".format(signature_file)):
        os.remove("./{}".format(signature_file))


if __name__ == "__main__":

    main(sys.argv)
