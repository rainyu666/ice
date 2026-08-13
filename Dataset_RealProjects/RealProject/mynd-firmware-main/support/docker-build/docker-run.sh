#!/bin/bash
#script must be run complete to clean up and fetch logs (don't use -e)
set -x

# this script must be executed from inside this directory to honor docker-config
# trace of the parameters passed from the buildagent:
##        cd ../..
##        mapvolume=$(pwd)
##        pathCheckout=%teamcity.build.checkoutDir%
##        cd ${pathCheckout}/support/docker-build
##        ./docker-run.sh projectname ${pathCheckout} ${mapvolume}
##
##

sourceDirectory=$2
shareDirectory=$3

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
# git descibe has an issue because of the repo owns by another user, therefore
# it fails in cmake scripts. In order to prevent it, we _mark_ the repo as safe. :)
echo "git config --global --add safe.directory \"$sourceDirectory\"" >> build.sh
echo "cmake -DTOOLCHAIN_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release -G Ninja .." >> build.sh
echo "ninja $1" >> build.sh

chmod +x build.sh

#fresh build
docker exec -i build-mcufirmware rm -rf ${shareDirectory}/build
mkdir -p ${sourceDirectory}/build

cp build.sh ${sourceDirectory}/build

docker exec -i build-mcufirmware ${sourceDirectory}/build/build.sh
errcode=$?
#save code for later, retrieve stuff and gracefully close

docker logs build-mcufirmware

# remove all (the image will not be destroyed, only the container layer)
docker container stop build-mcufirmware
docker container rm build-mcufirmware

if [ ${errcode} -ne 0 ]; then
    echo "docker reports error: " ${errcode}
    exit ${errcode}
fi

