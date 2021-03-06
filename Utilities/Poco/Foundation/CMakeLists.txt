set(LIBNAME "PocoFoundation")
#set(LIBNAMED "${LIBNAME}d")

set( BASE_SRCS 
  
  src/AtomicCounter.cpp
  
  src/ASCIIEncoding.cpp
  src/AbstractObserver.cpp
  src/ActiveDispatcher.cpp
  src/ArchiveStrategy.cpp
  src/AsyncChannel.cpp
  
  src/Base64Decoder.cpp
  src/Base64Encoder.cpp
  src/BinaryReader.cpp
  src/BinaryWriter.cpp
  src/Bugcheck.cpp
  src/ByteOrder.cpp
  src/Channel.cpp
  src/Checksum.cpp
  src/Condition.cpp
  src/Configurable.cpp
  src/ConsoleChannel.cpp
  src/CountingStream.cpp
  src/DateTime.cpp
  src/DateTimeFormat.cpp
  src/DateTimeFormatter.cpp
  src/DateTimeParser.cpp
  src/Debugger.cpp
  src/DeflatingStream.cpp
  src/DigestEngine.cpp
  src/DigestStream.cpp
  src/DirectoryIterator.cpp
  src/DynamicAny.cpp
  src/DynamicAnyHolder.cpp
  src/Environment.cpp
  src/ErrorHandler.cpp
  src/Event.cpp
  src/EventArgs.cpp
  src/Exception.cpp
  src/FPEnvironment.cpp
  src/File.cpp
  src/FileChannel.cpp
  src/FileStream.cpp
  src/FileStreamFactory.cpp
  src/Format.cpp
  src/Formatter.cpp
  src/FormattingChannel.cpp
  src/Glob.cpp
  src/Hash.cpp
  src/HashStatistic.cpp
  src/HexBinaryDecoder.cpp
  src/HexBinaryEncoder.cpp
  src/InflatingStream.cpp
  src/Latin1Encoding.cpp
  src/Latin9Encoding.cpp
  src/LineEndingConverter.cpp
  src/LocalDateTime.cpp
  src/LogFile.cpp
  src/LogStream.cpp
  src/Logger.cpp
  src/LoggingFactory.cpp
  src/LoggingRegistry.cpp
  src/MD2Engine.cpp
  src/MD4Engine.cpp
  src/MD5Engine.cpp
  src/Manifest.cpp
  src/MemoryPool.cpp
  src/Message.cpp
  src/Mutex.cpp
  src/NamedEvent.cpp
  src/NamedMutex.cpp
  src/NestedDiagnosticContext.cpp
  src/Notification.cpp
  src/NotificationCenter.cpp
  src/NotificationQueue.cpp
  src/TimedNotificationQueue.cpp
  src/PriorityNotificationQueue.cpp
  src/NullChannel.cpp
  src/NullStream.cpp
  src/NumberFormatter.cpp
  src/NumberParser.cpp
#  src/OpcomChannel.cpp  
  src/Path.cpp
  src/PatternFormatter.cpp
  src/Pipe.cpp
  src/PipeImpl.cpp
  src/PipeStream.cpp
  src/Process.cpp
  src/PurgeStrategy.cpp
  src/RWLock.cpp
  src/Random.cpp
  src/RandomStream.cpp
  src/RefCountedObject.cpp
  src/RegularExpression.cpp
  src/RotateStrategy.cpp
  src/Runnable.cpp
  src/SHA1Engine.cpp
  src/Semaphore.cpp
  src/SharedLibrary.cpp
  src/SharedMemory.cpp
  src/SignalHandler.cpp
  src/SimpleFileChannel.cpp
  src/SplitterChannel.cpp
  src/Stopwatch.cpp
  src/StreamChannel.cpp
  src/StreamConverter.cpp
  src/StreamCopier.cpp
  src/StreamTokenizer.cpp
  src/String.cpp
  src/StringTokenizer.cpp
  src/SynchronizedObject.cpp
  src/Task.cpp
  src/TaskManager.cpp
  src/TaskNotification.cpp
  src/TeeStream.cpp
  src/TemporaryFile.cpp
  src/TextConverter.cpp
  src/TextEncoding.cpp
  src/TextIterator.cpp
  src/Thread.cpp
  src/ThreadTarget.cpp
  src/ThreadLocal.cpp
  src/ThreadPool.cpp
  src/Timer.cpp
  src/Timespan.cpp
  src/Timestamp.cpp
  src/Timezone.cpp
  src/Token.cpp
  src/URI.cpp
  src/URIStreamFactory.cpp
  src/URIStreamOpener.cpp
  src/UTF16Encoding.cpp
  src/UTF8Encoding.cpp
  src/UTF8String.cpp
  src/UUID.cpp
  src/UUIDGenerator.cpp
  src/Unicode.cpp
  src/UnicodeConverter.cpp
#  src/Var.cpp
#  src/VarHolder.cpp
  src/Void.cpp
  src/Windows1252Encoding.cpp
  src/adler32.c
  src/compress.c
  src/crc32.c
  src/deflate.c
  src/gzio.c
  src/infback.c
  src/inffast.c
  src/inflate.c
  src/inftrees.c
  src/pcre_chartables.c
  src/pcre_compile.c
  src/pcre_exec.c
  src/pcre_fullinfo.c
  src/pcre_globals.c
  src/pcre_maketables.c
  src/pcre_newline.c
  src/pcre_ord2utf8.c
  src/pcre_study.c
  src/pcre_tables.c
  src/pcre_try_flipped.c
  src/pcre_ucd.c
  src/pcre_valid_utf8.c
  src/pcre_xclass.c
  src/pocomsg.mc
  src/trees.c
  src/zutil.c
)

set(WIN_SRCS
  src/EventLogChannel.cpp
  src/WindowsConsoleChannel.cpp
)

set(LIN_SRCS
  src/SyslogChannel.cpp
)

if(CMAKE_SYSTEM MATCHES "Windows")
 set(SRCS ${BASE_SRCS} ${WIN_SRCS})
 #add_definitions( -DPOCO_NO_FPENVIRONMENT -DPCRE_STATIC)
else(CMAKE_SYSTEM MATCHES "Windows")
 set(SRCS ${BASE_SRCS} ${LIN_SRCS})
 add_definitions( -DPCRE_STATIC)
endif(CMAKE_SYSTEM MATCHES "Windows")

if(CMAKE_COMPILER_IS_MINGW)
 add_definitions( -DPOCO_NO_FPENVIRONMENT -DPCRE_STATIC)
endif()

add_library( ${LIBNAME} ${LIB_MODE} ${SRCS} )
set_target_properties( ${LIBNAME} PROPERTIES
        DEFINE_SYMBOL Foundation_EXPORTS
  #COMPILE_FLAGS ${RELEASE_CXX_FLAGS}
	VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}
	SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
if(${CMAKE_CXX_COMPILER_ID} MATCHES "SunPro")
 set_target_properties( ${LIBNAME} PROPERTIES LINK_FLAGS "-library=stlport4")
endif(${CMAKE_CXX_COMPILER_ID} MATCHES "SunPro")
target_link_libraries( ${LIBNAME} ${SYSLIBS})

#add_library( ${LIBNAMED} ${LIB_MODE} ${SRCS} )
#set_target_properties( ${LIBNAMED}  
#	PROPERTIES COMPILE_FLAGS "${DEBUG_CXX_FLAGS}" 
#       DEFINE_SYMBOL Foundation_EXPORTS
#	VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}
#	SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
#if(${CMAKE_CXX_COMPILER_ID} MATCHES "SunPro")
# set_target_properties( ${LIBNAMED} PROPERTIES LINK_FLAGS "-library=stlport4")
#endif(${CMAKE_CXX_COMPILER_ID} MATCHES "SunPro")
#target_link_libraries( ${LIBNAMED} ${SYSLIBS})

