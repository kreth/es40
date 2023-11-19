# Docker Container Build

Do something like this:

### Ubuntu 20.04:

```
  docker build -t es40.ubuntu20 --progress plain Dockerfile.ubuntu18
  docker run -it -v <local_artefacts_dir>:/import -p21264:21264 es40.ubuntu20 /bin/bash
```


### Ubuntu 18.04 LTS:
```
  docker build -t es40.ubuntu18 --progress plain Dockerfile.ubuntu20
  docker run -it -v <local_artefacts_dir>:/import -p21264:21264 es40.ubuntu18 /bin/bash
```
