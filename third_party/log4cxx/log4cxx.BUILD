load("@bazel_skylib//rules:copy_file.bzl", "copy_file")

copy_file(
    name = "log4cxx_h",
    src = "@inf_bmq//third_party/log4cxx:log4cxx.h",
    out = "src/main/include/log4cxx/log4cxx.h",
)

copy_file(
    name = "log4cxx_private_h",
    src = "@inf_bmq//third_party/log4cxx:log4cxx_private.h",
    out = "src/main/include/log4cxx/private/log4cxx_private.h",
)

cc_library(
    name = "log4cxx",
    srcs = [
        "src/main/cpp/action.cpp",
        "src/main/cpp/appenderattachableimpl.cpp",
        "src/main/cpp/appenderskeleton.cpp",
        "src/main/cpp/aprinitializer.cpp",
        "src/main/cpp/asyncappender.cpp",
        "src/main/cpp/basicconfigurator.cpp",
        "src/main/cpp/bufferedwriter.cpp",
        "src/main/cpp/bytearrayinputstream.cpp",
        "src/main/cpp/bytearrayoutputstream.cpp",
        "src/main/cpp/bytebuffer.cpp",
        "src/main/cpp/cacheddateformat.cpp",
        "src/main/cpp/charsetdecoder.cpp",
        "src/main/cpp/charsetencoder.cpp",
        "src/main/cpp/class.cpp",
        "src/main/cpp/classnamepatternconverter.cpp",
        "src/main/cpp/classregistration.cpp",
        "src/main/cpp/condition.cpp",
        "src/main/cpp/configurator.cpp",
        "src/main/cpp/consoleappender.cpp",
        "src/main/cpp/cyclicbuffer.cpp",
        "src/main/cpp/dailyrollingfileappender.cpp",
        "src/main/cpp/datagrampacket.cpp",
        "src/main/cpp/datagramsocket.cpp",
        "src/main/cpp/date.cpp",
        "src/main/cpp/dateformat.cpp",
        "src/main/cpp/datelayout.cpp",
        "src/main/cpp/datepatternconverter.cpp",
        "src/main/cpp/defaultconfigurator.cpp",
        "src/main/cpp/defaultloggerfactory.cpp",
        "src/main/cpp/defaultrepositoryselector.cpp",
        "src/main/cpp/domconfigurator.cpp",
        "src/main/cpp/exception.cpp",
        "src/main/cpp/fallbackerrorhandler.cpp",
        "src/main/cpp/file.cpp",
        "src/main/cpp/fileappender.cpp",
        "src/main/cpp/filedatepatternconverter.cpp",
        "src/main/cpp/fileinputstream.cpp",
        "src/main/cpp/filelocationpatternconverter.cpp",
        "src/main/cpp/fileoutputstream.cpp",
        "src/main/cpp/filerenameaction.cpp",
        "src/main/cpp/filewatchdog.cpp",
        "src/main/cpp/filter.cpp",
        "src/main/cpp/filterbasedtriggeringpolicy.cpp",
        "src/main/cpp/fixedwindowrollingpolicy.cpp",
        "src/main/cpp/formattinginfo.cpp",
        "src/main/cpp/fulllocationpatternconverter.cpp",
        "src/main/cpp/gzcompressaction.cpp",
        "src/main/cpp/hierarchy.cpp",
        "src/main/cpp/htmllayout.cpp",
        "src/main/cpp/inetaddress.cpp",
        "src/main/cpp/inputstream.cpp",
        "src/main/cpp/inputstreamreader.cpp",
        "src/main/cpp/integer.cpp",
        "src/main/cpp/integerpatternconverter.cpp",
        "src/main/cpp/layout.cpp",
        "src/main/cpp/level.cpp",
        "src/main/cpp/levelmatchfilter.cpp",
        "src/main/cpp/levelpatternconverter.cpp",
        "src/main/cpp/levelrangefilter.cpp",
        "src/main/cpp/linelocationpatternconverter.cpp",
        "src/main/cpp/lineseparatorpatternconverter.cpp",
        "src/main/cpp/literalpatternconverter.cpp",
        "src/main/cpp/loader.cpp",
        "src/main/cpp/locale.cpp",
        "src/main/cpp/locationinfo.cpp",
        "src/main/cpp/logger.cpp",
        "src/main/cpp/loggerpatternconverter.cpp",
        "src/main/cpp/loggingevent.cpp",
        "src/main/cpp/loggingeventpatternconverter.cpp",
        "src/main/cpp/loglog.cpp",
        "src/main/cpp/logmanager.cpp",
        "src/main/cpp/logstream.cpp",
        "src/main/cpp/manualtriggeringpolicy.cpp",
        "src/main/cpp/mdc.cpp",
        "src/main/cpp/messagebuffer.cpp",
        "src/main/cpp/messagepatternconverter.cpp",
        "src/main/cpp/methodlocationpatternconverter.cpp",
        "src/main/cpp/mutex.cpp",
        "src/main/cpp/nameabbreviator.cpp",
        "src/main/cpp/namepatternconverter.cpp",
        "src/main/cpp/ndc.cpp",
        "src/main/cpp/ndcpatternconverter.cpp",
        "src/main/cpp/nteventlogappender.cpp",
        "src/main/cpp/objectimpl.cpp",
        "src/main/cpp/objectoutputstream.cpp",
        "src/main/cpp/objectptr.cpp",
        "src/main/cpp/obsoleterollingfileappender.cpp",
        "src/main/cpp/odbcappender.cpp",
        "src/main/cpp/onlyonceerrorhandler.cpp",
        "src/main/cpp/optionconverter.cpp",
        "src/main/cpp/outputdebugstringappender.cpp",
        "src/main/cpp/outputstream.cpp",
        "src/main/cpp/outputstreamwriter.cpp",
        "src/main/cpp/patternconverter.cpp",
        "src/main/cpp/patternlayout.cpp",
        "src/main/cpp/patternparser.cpp",
        "src/main/cpp/pool.cpp",
        "src/main/cpp/properties.cpp",
        "src/main/cpp/propertiespatternconverter.cpp",
        "src/main/cpp/propertyconfigurator.cpp",
        "src/main/cpp/propertyresourcebundle.cpp",
        "src/main/cpp/propertysetter.cpp",
        "src/main/cpp/reader.cpp",
        "src/main/cpp/relativetimedateformat.cpp",
        "src/main/cpp/relativetimepatternconverter.cpp",
        "src/main/cpp/resourcebundle.cpp",
        "src/main/cpp/rollingfileappender.cpp",
        "src/main/cpp/rollingpolicy.cpp",
        "src/main/cpp/rollingpolicybase.cpp",
        "src/main/cpp/rolloverdescription.cpp",
        "src/main/cpp/rootlogger.cpp",
        "src/main/cpp/serversocket.cpp",
        "src/main/cpp/simpledateformat.cpp",
        "src/main/cpp/simplelayout.cpp",
        "src/main/cpp/sizebasedtriggeringpolicy.cpp",
        "src/main/cpp/smtpappender.cpp",
        "src/main/cpp/socket.cpp",
        "src/main/cpp/socketappender.cpp",
        "src/main/cpp/socketappenderskeleton.cpp",
        "src/main/cpp/sockethubappender.cpp",
        "src/main/cpp/socketoutputstream.cpp",
        "src/main/cpp/strftimedateformat.cpp",
        "src/main/cpp/stringhelper.cpp",
        "src/main/cpp/stringmatchfilter.cpp",
        "src/main/cpp/stringtokenizer.cpp",
        "src/main/cpp/synchronized.cpp",
        "src/main/cpp/syslogappender.cpp",
        "src/main/cpp/syslogwriter.cpp",
        "src/main/cpp/system.cpp",
        "src/main/cpp/systemerrwriter.cpp",
        "src/main/cpp/systemoutwriter.cpp",
        "src/main/cpp/telnetappender.cpp",
        "src/main/cpp/threadcxx.cpp",
        "src/main/cpp/threadlocal.cpp",
        "src/main/cpp/threadpatternconverter.cpp",
        "src/main/cpp/threadspecificdata.cpp",
        "src/main/cpp/throwableinformationpatternconverter.cpp",
        "src/main/cpp/timebasedrollingpolicy.cpp",
        "src/main/cpp/timezone.cpp",
        "src/main/cpp/transcoder.cpp",
        "src/main/cpp/transform.cpp",
        "src/main/cpp/triggeringpolicy.cpp",
        "src/main/cpp/ttcclayout.cpp",
        "src/main/cpp/writer.cpp",
        "src/main/cpp/writerappender.cpp",
        "src/main/cpp/xmllayout.cpp",
        "src/main/cpp/xmlsocketappender.cpp",
        "src/main/cpp/zipcompressaction.cpp",
        ":log4cxx_h",
        ":log4cxx_private_h",
    ],
    hdrs = glob(["**/*.h"]),
    defines = [
        "_GNU_SOURCE",
        "_REENTRANT",
        "HAVE_DLFCN_H=1",
        "HAVE_FWIDE=1",
        "HAVE_INTTYPES_H=1",
        "HAVE_MBSRTOWCS=1",
        "HAVE_MEMORY_H=1",
        "HAVE_STDINT_H=1",
        "HAVE_STDLIB_H=1",
        "HAVE_STRING_H=1",
        "HAVE_STRINGS_H=1",
        "HAVE_SYS_STAT_H=1",
        "HAVE_SYS_TYPES_H=1",
        "HAVE_SYSLOG=1",
        "HAVE_UNISTD_H=1",
        "HAVE_WCSTOMBS=1",
        "LINUX",
        "PACKAGE_BUGREPORT=\"\"",
        "PACKAGE_NAME=\"\"",
        "PACKAGE_STRING=\"\"",
        "PACKAGE_TARNAME=\"\"",
        "PACKAGE_VERSION=\"\"",
        "PACKAGE=\"log4cxx\"",
        "STDC_HEADERS=1",
        "VERSION=\"0.10.0\"",
    ],
    includes = [
        "./src/main/include",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@org_apache_apr//:apr",
        "@org_apache_apr_util//:apr_util",
    ],
)
