#!/bin/bash
set -x

# this script must be executed from inside this directory to honor docker-config
# trace of the parameters passed from the buildagent:
##        cd ../..
##        mapvolume=$(pwd)
##        pathCheckout=%teamcity.build.checkoutDir%
##        cd ${pathCheckout}/support/docker-build
##        ./docker-unittest.sh ${pathCheckout} ${mapvolume} <list of projects>
##
##

sourceDirectory=$1
shareDirectory=$2

#stop and rm old container if any
docker container stop build-mcufirmware
docker container rm build-mcufirmware

docker build -t build-mcufirmware docker-config

#we need to share more than the checkout directory but also the git mirror has to be mapped to the same directory of the host
#i.e. /home/buildagent/work/mcufirmware and /home/buildagent/system/git have common /home/buildagent
# we are using the deprecated --volume share because some buildagents do not have docker updated

docker run -d -it --name build-mcufirmware --volume ${shareDirectory}:${shareDirectory}:rw build-mcufirmware
docker ps -a

echo "creating buildscript that will run in docker container"

echo "#!/bin/sh" > build.sh
echo "cd ${sourceDirectory}/build" >> build.sh
echo "cmake -DTOOLCHAIN_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release .." >> build.sh
argc=$#
argv=("$@")

for (( j=2; j<argc; j++ )); do
    echo "make ${argv[j]}" >> build.sh
done

chmod +x build.sh

#fresh build
rm -rf ${sourceDirectory}/build
mkdir ${sourceDirectory}/build
cp build.sh ${sourceDirectory}/build

docker exec build-mcufirmware ${sourceDirectory}/build/build.sh

docker logs build-mcufirmware

# remove all (the image will not be destroyed, only the container layer)
docker container stop build-mcufirmware
docker container rm build-mcufirmware
