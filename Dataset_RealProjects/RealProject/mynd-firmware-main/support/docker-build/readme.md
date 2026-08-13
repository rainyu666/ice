# Docker hints


## Mac OSx
Install docker with brew
```
brew install docker-machine
docker-machine create --driver virtualbox default
docker-machine restart
eval $(docker-machine env)
```

running in terminal start
```
docker-machine start
eval $(docker-machine env)
```
