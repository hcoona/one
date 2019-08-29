# INI Configuration Flattener #

This project help to flatten INI file according to several dimensions.

## Getting Started ##

### How to build ###

```bash
bazel build //ini_configuration_flattener_cli/src/main:ini_configuration_flattener_deploy.jar
```

bazel would pack all dependencies into the single output JAR package located at
`./bazel-bin/ini_configuration_flattener_cli/src/main/ini_configuration_flattener_deploy.jar`.

### How to use ###

```bash
java -DinputFile=<input file path> -DoutputFile=<output file path> -jar ini_configuration_flattener_deploy.jar
```

For dev usage, please add `-Ddev=true` to skip load machine info file.

Both input & output file must be a INI file. If the key contains symbol `$`,
we call it an overwrite key, else call it a normal key.

The value from normal key is chosen only if there is no overwrite key matched.
The first matched overwrite key wins.

A overwrite key matches only when all its the dimensional values match.

## Example ##

Assume the environment attribute is as following:

```ini
Region=CN
DC=lf
Cluster=default
IP=10.21.151.40
MT=RM
SKU=P-I7-48-256-4x1T
ScaleUnit=1
```

Given input file as following:

```ini
; All machines in CN depends on the same HDFS
Region:CN$hdfs=hdfs://haruna/

; All machines in the same DataCenter share the same zookeeper
; cn-flink use separate ZK to provide higher isolation level
DC:lf,Cluster:flink$zk=zk://lf-flink/
DC:lf$zk=zk://lf/

ha.ids=rm1,rm2
IP:10.21.151.40$ha.id=rm1
IP:10.21.151.41$ha.id=rm2

SKU:P-I7-48-256-4x1T$disk=/data00,/data01,/data02,/data03
SKU:P-I7-48-256-2x1T-S$disk=/data00,/data01

; Training usage machine fix to 20 vcores for 48 cores machine
MT:NM_ps,SKU:P-I7-48-256-4x1T$cpu.vcores=20
SKU:P-I7-48-256-4x1T$cpu.vcores=47
```

The output file should be:

```ini
hdfs=hdfs://haruna/
zk=zk://lf/
ha.ids=rm1,rm2
ha.id=rm1
disk=/data00,/data01,/data02,/data03
cpu.vcores=47
```

## How it works ##

### Dimensions ###

It's designed to load all dimensions from a CSV file.

## Future ##

There are several work items still need to fill:

1. Add abstraction of configuration instead of using `Map<...>` directly.
2. Define a collection of standard attributes, load them from standard position.
    Allow user to define extra attributes & load them from specific position.

## How to contribute ##

The `master` branch is always stable version. The formal released version is tagged.
The `dev` branch is active developing branch.

Please submit Merge Request to `dev` branch. Someday later, 
the Master decide to merge it to `master` branch,
bump the version, tag release, etc.
