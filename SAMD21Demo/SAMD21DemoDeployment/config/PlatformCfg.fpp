# =======================================================================
# FPP file for configuration of various F Prime Platform and Os constants
# =======================================================================

# Handles for the console, task, mutex, raw time, CPU, memory and file system are cut to the size of the
# implementations used here (Arduino console and raw time, fprime-baremetal task, CPU and memory, stub mutex). Every
# Os::Mutex and Os::RawTime holds one, so this saves about 500 bytes of RAM. F´ checks each implementation's size at
# compile time, so one that doesn't fit fails to build until its size is raised.

@ Maximum size of a handle for Os::Console
constant FW_CONSOLE_HANDLE_MAX_SIZE = 8

@ Maximum size of a handle for Os::Task
constant FW_TASK_HANDLE_MAX_SIZE = 16

@ Maximum size of a handle for Os::File
constant FW_FILE_HANDLE_MAX_SIZE = 48

@ Maximum size of a handle for Os::Mutex
constant FW_MUTEX_HANDLE_MAX_SIZE = 8

@ Maximum size of a handle for Os::Queue
constant FW_QUEUE_HANDLE_MAX_SIZE = 368

@ Maximum size of a handle for Os::Directory
constant FW_DIRECTORY_HANDLE_MAX_SIZE = 48

@ Maximum size of a handle for Os::FileSystem
constant FW_FILESYSTEM_HANDLE_MAX_SIZE = 8

@ Maximum size of a handle for Os::RawTime
constant FW_RAW_TIME_HANDLE_MAX_SIZE = 16

@ Maximum allowed serialization size for Os::RawTime objects
constant FW_RAW_TIME_SERIALIZATION_MAX_SIZE = 8

@ Maximum size of a handle for Os::CountingSemaphore
constant FW_COUNTING_SEMAPHORE_HANDLE_MAX_SIZE = 48

@ Maximum size of a handle for Os::ConditionVariable
constant FW_CONDITION_VARIABLE_HANDLE_MAX_SIZE = 56

@ Maximum size of a handle for Os::Cpu
constant FW_CPU_HANDLE_MAX_SIZE = 8

@ Maximum size of a handle for Os::Memory
constant FW_MEMORY_HANDLE_MAX_SIZE = 8

@ Alignment of handle storage
constant FW_HANDLE_ALIGNMENT = 8

@ Chunk size for working with files in the OSAL layer
constant FW_FILE_CHUNK_SIZE = 512
