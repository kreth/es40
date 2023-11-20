# Docker Build Container for ES40

Do something like this:

### Ubuntu 18.04 LTS:
```
  docker build -t es40.ubuntu18 --progress plain Dockerfile.ubuntu18
  docker run -it -p21264:21264 es40.ubuntu18 /bin/bash
```

### Ubuntu 20.04:
```
  docker build -t es40.ubuntu20 --progress plain Dockerfile.ubuntu20
  docker run -it -p21264:21264 es40.ubuntu20 /bin/bash
```

Currently there are Problems with Sensing the SCSI Disk / IO Read Errors on Ubuntu 18.04
Have to figure out what is going on here... ;-)
