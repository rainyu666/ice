#Logger utility

This module is a simple logger implementation that can be configured to taste.

## Intro

1. Logger provides a simple API to log messages.
2. Logger has two types of configuration: config and format. Config is used to configure the logger itself, while format is used to configure the format of the logs.
3. Logger supports cmake configuration, which defined in FindLogger.cmake (please, see the cmake section below).
4. Logger is simple and can provide only two options: full/colorized output and/or raw printf implementation.
5. Logger _optionally_ provides syscalls implementation/overriders for the `write()` and `write_r()` syscalls.
6. Logger _optionally_ provides a `PUTCHAR()` implementation as well.

Using the logger from interrupts with a priority higher than `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` is NOT safe.

## Set up

There are two ways to set up the logger: using CMake or manually.

### Manual set up

1. Include logger's source files in your project.
2. Include logger's header files in your project.
3. You need to provide a source file that implements the `PUTCHAR()` function (see `include/logger_putchar.h`),
4. _Optionally_ override `logger_get_timestamp()`, `logger_init()`, and `logger_flush()` function (see `implementations/logger_implementation.h`).
5. Use one of existing configs and formats, or copy those header files from `configs/` and `formats/` directories somewhere to your own project and adjust as needed.

### Using CMake

In order to use cmake, please follow the example below:

```cmake
# Provide the path to the logger's cmake file
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/teufel/libs")

# Find the logger package
find_package(Logger REQUIRED)

# Link the logger library
target_link_libraries(${PROJECT_NAME} PRIVATE
    Logger
    Logger::Config1 # Optional configuration file, which defined in configs/num1/logger_config.h
    Logger::Format1 # Optional format file, which defined in formats/num1/logger_format.h
)
```

After that, all you need to do is to include the logger.h header file in your project.

Note that the 2 definitions must precede the `#include` clause.
The definitions are used as follows:
- `LOG_MODULE_NAME`: Defines the name by which this logged module is identified. The log is prefixed with this string if `LOGGER_PRINT_LOG_LOCATION` is set to 1 in `logger_config.h`.
- `LOG_LEVEL`: Defines the logging level enabled for this logged module. Any logs above the defined log level will not be present in the binary.

## Configurations and formats

The logger includes a few configuration/format files in:
`configs/num1/logger_config.h`
`configs/num2/logger_config.h`
`formats/num1/logger_format.h`
`formats/num2/logger_format.h`

**DO NOT EDIT THESE FILES IF YOU DON'T EXACTLY KNOW WHERE THEY ARE USED. INSTEAD, FEEL FREE TO ADD YOUR OWN CONFIGS/FORMATS!**

However, feel free to adjust the default configuration _*for every project*_ if a different configuration is a more sensible default. This should be discussed with the team beforehand, but it shouldn't be a big issue since every project should have configuration files of its own.

### Notable configuration options

**Logger API function name customization**:
It is possible to customize the logger function names by renaming the default macro names.
For example, easily rename `log_debug()` to `dbg_log()` if you so desire.

**Log header/prefix customization**:
Customize the information printed with each log by default. You can choose to print the location of each log, timestamp, log level, etc.

**New line customization**:
Customize the end of line character sequence. This is useful to set up the logger to finish every log "\r\n", "\n" or nothing at all if
you prefer to explicitly add the new line sequence yourself.

**Force logging level globally**:
You can force changing the log level of the entire project by changing one line of code, rather than
adjusting log levels per module. This is useful for example if you want to log the entire project with all logs. Without this option you
would have to set the log level of each module to `LOG_LEVEL_TRACE`.
