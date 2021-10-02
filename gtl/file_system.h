/* Copyright 2015 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef GTL_FILE_SYSTEM_H_
#define GTL_FILE_SYSTEM_H_

#include <stdint.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "gtl/file_statistics.h"
#include "gtl/macros.h"

#ifdef OS_WIN
#undef DeleteFile
#undef CopyFile
#undef TranslateName
#endif

namespace gtl {

class RandomAccessFile;
class ReadOnlyMemoryRegion;
class WritableFile;

/// A generic interface for accessing a file system.  Implementations
/// of custom filesystem adapters must implement this interface,
/// RandomAccessFile, WritableFile, and ReadOnlyMemoryRegion classes.
class FileSystem {
 public:
  /// \brief Creates a brand new random access read-only file with the
  /// specified name.
  ///
  /// On success, stores a pointer to the new file in
  /// *result and returns OK.  On failure stores NULL in *result and
  /// returns non-OK.  If the file does not exist, returns a non-OK
  /// status.
  ///
  /// The returned file may be concurrently accessed by multiple threads.
  ///
  /// The ownership of the returned RandomAccessFile is passed to the caller
  /// and the object should be deleted when is not used.
  virtual absl::Status NewRandomAccessFile(
      const std::string& fname, std::unique_ptr<RandomAccessFile>* result) = 0;

  /// \brief Creates an object that writes to a new file with the specified
  /// name.
  ///
  /// Deletes any existing file with the same name and creates a
  /// new file.  On success, stores a pointer to the new file in
  /// *result and returns OK.  On failure stores NULL in *result and
  /// returns non-OK.
  ///
  /// The returned file will only be accessed by one thread at a time.
  ///
  /// The ownership of the returned WritableFile is passed to the caller
  /// and the object should be deleted when is not used.
  virtual absl::Status NewWritableFile(
      const std::string& fname, std::unique_ptr<WritableFile>* result) = 0;

  /// \brief Creates an object that either appends to an existing file, or
  /// writes to a new file (if the file does not exist to begin with).
  ///
  /// On success, stores a pointer to the new file in *result and
  /// returns OK.  On failure stores NULL in *result and returns
  /// non-OK.
  ///
  /// The returned file will only be accessed by one thread at a time.
  ///
  /// The ownership of the returned WritableFile is passed to the caller
  /// and the object should be deleted when is not used.
  virtual absl::Status NewAppendableFile(
      const std::string& fname, std::unique_ptr<WritableFile>* result) = 0;

  /// \brief Creates a readonly region of memory with the file context.
  ///
  /// On success, it returns a pointer to read-only memory region
  /// from the content of file fname. The ownership of the region is passed to
  /// the caller. On failure stores nullptr in *result and returns non-OK.
  ///
  /// The returned memory region can be accessed from many threads in parallel.
  ///
  /// The ownership of the returned ReadOnlyMemoryRegion is passed to the caller
  /// and the object should be deleted when is not used.
  virtual absl::Status NewReadOnlyMemoryRegionFromFile(
      const std::string& fname, std::unique_ptr<ReadOnlyMemoryRegion>* result) = 0;

  /// Returns OK if the named path exists and NOT_FOUND otherwise.
  virtual absl::Status FileExists(const std::string& fname) = 0;

  /// Returns true if all the listed files exist, false otherwise.
  /// if status is not null, populate the vector with a detailed status
  /// for each file.
  virtual bool FilesExist(const std::vector<std::string>& files,
                          std::vector<absl::Status>* status);

  /// \brief Returns the immediate children in the given directory.
  ///
  /// The returned paths are relative to 'dir'.
  virtual absl::Status GetChildren(const std::string& dir,
                                         std::vector<std::string>* result) = 0;

  /// \brief Given a pattern, stores in *results the set of paths that matches
  /// that pattern. *results is cleared.
  ///
  /// pattern must match all of a name, not just a substring.
  ///
  /// pattern: { term }
  /// term:
  ///   '*': matches any sequence of non-'/' characters
  ///   '?': matches a single non-'/' character
  ///   '[' [ '^' ] { match-list } ']':
  ///        matches any single character (not) on the list
  ///   c: matches character c (c != '*', '?', '\\', '[')
  ///   '\\' c: matches character c
  /// character-range:
  ///   c: matches character c (c != '\\', '-', ']')
  ///   '\\' c: matches character c
  ///   lo '-' hi: matches character c for lo <= c <= hi
  ///
  /// Typical return codes:
  ///  * OK - no errors
  ///  * UNIMPLEMENTED - Some underlying functions (like GetChildren) are not
  ///                    implemented
  virtual absl::Status GetMatchingPaths(const std::string& pattern,
                                        std::vector<std::string>* results) = 0;

  /// \brief Checks if the given filename matches the pattern.
  ///
  /// This function provides the equivalent of posix fnmatch, however it is
  /// implemented without fnmatch to ensure that this can be used for cloud
  /// filesystems on windows. For windows filesystems, it uses PathMatchSpec.
  virtual bool Match(const std::string& filename, const std::string& pattern);

  /// \brief Obtains statistics for the given path.
  virtual absl::Status Stat(const std::string& fname,
                                  FileStatistics* stat) = 0;

  /// \brief Deletes the named file.
  virtual absl::Status DeleteFile(const std::string& fname) = 0;

  /// \brief Creates the specified directory.
  /// Typical return codes:
  ///  * OK - successfully created the directory.
  ///  * ALREADY_EXISTS - directory with name dirname already exists.
  ///  * PERMISSION_DENIED - dirname is not writable.
  virtual absl::Status CreateDir(const std::string& dirname) = 0;

  /// \brief Creates the specified directory and all the necessary
  /// subdirectories.
  /// Typical return codes:
  ///  * OK - successfully created the directory and sub directories, even if
  ///         they were already created.
  ///  * PERMISSION_DENIED - dirname or some subdirectory is not writable.
  virtual absl::Status RecursivelyCreateDir(const std::string& dirname);

  /// \brief Deletes the specified directory.
  virtual absl::Status DeleteDir(const std::string& dirname) = 0;

  /// \brief Deletes the specified directory and all subdirectories and files
  /// underneath it. This is accomplished by traversing the directory tree
  /// rooted at dirname and deleting entries as they are encountered.
  ///
  /// If dirname itself is not readable or does not exist, *undeleted_dir_count
  /// is set to 1, *undeleted_file_count is set to 0 and an appropriate status
  /// (e.g. NOT_FOUND) is returned.
  ///
  /// If dirname and all its descendants were successfully deleted, TF_OK is
  /// returned and both error counters are set to zero.
  ///
  /// Otherwise, while traversing the tree, undeleted_file_count and
  /// undeleted_dir_count are updated if an entry of the corresponding type
  /// could not be deleted. The returned error status represents the reason that
  /// any one of these entries could not be deleted.
  ///
  /// REQUIRES: undeleted_files, undeleted_dirs to be not null.
  ///
  /// Typical return codes:
  ///  * OK - dirname exists and we were able to delete everything underneath.
  ///  * NOT_FOUND - dirname doesn't exist
  ///  * PERMISSION_DENIED - dirname or some descendant is not writable
  ///  * UNIMPLEMENTED - Some underlying functions (like Delete) are not
  ///                    implemented
  virtual absl::Status DeleteRecursively(const std::string& dirname,
                                         int64_t* undeleted_files,
                                         int64_t* undeleted_dirs);

  /// \brief Stores the size of `fname` in `*file_size`.
  virtual absl::Status GetFileSize(const std::string& fname,
                                   uint64_t* file_size) = 0;

  /// \brief Overwrites the target if it exists.
  virtual absl::Status RenameFile(const std::string& src,
                                  const std::string& target) = 0;

  /// \brief Copy the src to target.
  virtual absl::Status CopyFile(const std::string& src, const std::string& target);

  /// \brief Translate an URI to a filename for the FileSystem implementation.
  ///
  /// The implementation in this class cleans up the path, removing
  /// duplicate /'s, resolving .. and removing trailing '/'.
  /// This respects relative vs. absolute paths, but does not
  /// invoke any system calls (getcwd(2)) in order to resolve relative
  /// paths with respect to the actual working directory.  That is, this is
  /// purely std::string manipulation, completely independent of process state.
  virtual std::string TranslateName(const std::string& name) const;

  /// \brief Returns whether the given path is a directory or not.
  ///
  /// Typical return codes (not guaranteed exhaustive):
  ///  * OK - The path exists and is a directory.
  ///  * FAILED_PRECONDITION - The path exists and is not a directory.
  ///  * NOT_FOUND - The path entry does not exist.
  ///  * PERMISSION_DENIED - Insufficient permissions.
  ///  * UNIMPLEMENTED - The file factory doesn't support directories.
  virtual absl::Status IsDirectory(const std::string& fname);

  /// \brief Returns whether the given path is on a file system
  /// that has atomic move capabilities. This can be used
  /// to determine if there needs to be a temp location to safely write objects.
  /// The second boolean argument has_atomic_move contains this information.
  ///
  /// Returns one of the following status codes (not guaranteed exhaustive):
  ///  * OK - The path is on a recognized file system,
  ///         so has_atomic_move holds the above information.
  ///  * UNIMPLEMENTED - The file system of the path hasn't been implemented in
  ///  TF
  virtual absl::Status HasAtomicMove(const std::string& path, bool* has_atomic_move);

  /// \brief Flushes any cached filesystem objects from memory.
  virtual void FlushCaches();

  /// \brief The separator this filesystem uses.
  ///
  /// This is implemented as a part of the filesystem, because even on windows,
  /// a user may need access to filesystems with '/' separators, such as cloud
  /// filesystems.
  virtual char Separator() const;

  /// \brief Split a path to its basename and dirname.
  ///
  /// Helper function for Basename and Dirname.
  std::pair<absl::string_view, absl::string_view> SplitPath(absl::string_view uri) const;

  /// \brief returns the final file name in the given path.
  ///
  /// Returns the part of the path after the final "/".  If there is no
  /// "/" in the path, the result is the same as the input.
  virtual absl::string_view Basename(absl::string_view path) const;

  /// \brief Returns the part of the path before the final "/".
  ///
  /// If there is a single leading "/" in the path, the result will be the
  /// leading "/".  If there is no "/" in the path, the result is the empty
  /// prefix of the input.
  absl::string_view Dirname(absl::string_view path) const;

  /// \brief Returns the part of the basename of path after the final ".".
  ///
  /// If there is no "." in the basename, the result is empty.
  absl::string_view Extension(absl::string_view path) const;

  /// \brief Clean duplicate and trailing, "/"s, and resolve ".." and ".".
  ///
  /// NOTE: This respects relative vs. absolute paths, but does not
  /// invoke any system calls (getcwd(2)) in order to resolve relative
  /// paths with respect to the actual working directory.  That is, this is
  /// purely std::string manipulation, completely independent of process state.
  std::string CleanPath(absl::string_view path) const;

  /// \brief Creates a URI from a scheme, host, and path.
  ///
  /// If the scheme is empty, we just return the path.
  std::string CreateURI(absl::string_view scheme, absl::string_view host,
                   absl::string_view path) const;

  ///  \brief Creates a temporary file name with an extension.
  std::string GetTempFilename(const std::string& extension) const;

  /// \brief Return true if path is absolute.
  bool IsAbsolutePath(absl::string_view path) const;

#ifndef SWIG  // variadic templates
  /// \brief Join multiple paths together.
  ///
  /// This function also removes the unnecessary path separators.
  /// For example:
  ///
  ///  Arguments                  | JoinPath
  ///  ---------------------------+----------
  ///  '/foo', 'bar'              | /foo/bar
  ///  '/foo/', 'bar'             | /foo/bar
  ///  '/foo', '/bar'             | /foo/bar
  ///
  /// Usage:
  /// std::string path = io::JoinPath("/mydir", filename);
  /// std::string path = io::JoinPath(FLAGS_test_srcdir, filename);
  /// std::string path = io::JoinPath("/full", "path", "to", "filename");
  template <typename... T>
  std::string JoinPath(const T&... args) {
    return JoinPathImpl({args...});
  }
#endif /* SWIG */

  std::string JoinPathImpl(std::initializer_list<absl::string_view> paths);

  /// \brief Populates the scheme, host, and path from a URI.
  ///
  /// scheme, host, and path are guaranteed by this function to point into the
  /// contents of uri, even if empty.
  ///
  /// Corner cases:
  /// - If the URI is invalid, scheme and host are set to empty std::strings and the
  ///  passed std::string is assumed to be a path
  /// - If the URI omits the path (e.g. file://host), then the path is left
  /// empty.
  void ParseURI(absl::string_view remaining, absl::string_view* scheme, absl::string_view* host,
                absl::string_view* path) const;

  FileSystem() = default;

  virtual ~FileSystem() = default;
};

/// A file abstraction for randomly reading the contents of a file.
class RandomAccessFile {
 public:
  RandomAccessFile() {}
  virtual ~RandomAccessFile() = default;

  /// \brief Returns the name of the file.
  ///
  /// This is an optional operation that may not be implemented by every
  /// filesystem.
  virtual absl::Status Name(absl::string_view* result) const {
    ignore_result(result);
    return absl::UnimplementedError("This filesystem does not support Name()");
  }

  /// \brief Reads up to `n` bytes from the file starting at `offset`.
  ///
  /// `scratch[0..n-1]` may be written by this routine.  Sets `*result`
  /// to the data that was read (including if fewer than `n` bytes were
  /// successfully read).  May set `*result` to point at data in
  /// `scratch[0..n-1]`, so `scratch[0..n-1]` must be live when
  /// `*result` is used.
  ///
  /// On OK returned status: `n` bytes have been stored in `*result`.
  /// On non-OK returned status: `[0..n]` bytes have been stored in `*result`.
  ///
  /// Returns `OUT_OF_RANGE` if fewer than n bytes were stored in `*result`
  /// because of EOF.
  ///
  /// Safe for concurrent use by multiple threads.
  virtual absl::Status Read(uint64_t offset, size_t n, absl::string_view* result,
                                  char* scratch) const = 0;

  /// \brief Read up to `n` bytes from the file starting at `offset`.
  virtual absl::Status Read(uint64_t offset, size_t n,
                                  absl::Cord* cord) const {
    ignore_result(offset);
    ignore_result(n);
    ignore_result(cord);

    return absl::UnimplementedError(
        "Read(uint64_t, size_t, absl::Cord*) is not "
        "implemented");
  }

 private:
  RandomAccessFile(const RandomAccessFile&) = delete;
  RandomAccessFile& operator=(const RandomAccessFile&) = delete;
};

/// \brief A file abstraction for sequential writing.
///
/// The implementation must provide buffering since callers may append
/// small fragments at a time to the file.
class WritableFile {
 public:
  WritableFile() {}
  virtual ~WritableFile() = default;

  /// \brief Append 'data' to the file.
  virtual absl::Status Append(absl::string_view data) = 0;

  // \brief Append 'data' to the file.
  virtual absl::Status Append(const absl::Cord& cord) {
    ignore_result(cord);
    return absl::UnimplementedError("Append(absl::Cord) is not implemented");
  }

  /// \brief Close the file.
  ///
  /// Flush() and de-allocate resources associated with this file
  ///
  /// Typical return codes (not guaranteed to be exhaustive):
  ///  * OK
  ///  * Other codes, as returned from Flush()
  virtual absl::Status Close() = 0;

  /// \brief Flushes the file and optionally syncs contents to filesystem.
  ///
  /// This should flush any local buffers whose contents have not been
  /// delivered to the filesystem.
  ///
  /// If the process terminates after a successful flush, the contents
  /// may still be persisted, since the underlying filesystem may
  /// eventually flush the contents.  If the OS or machine crashes
  /// after a successful flush, the contents may or may not be
  /// persisted, depending on the implementation.
  virtual absl::Status Flush() = 0;

  // \brief Returns the name of the file.
  ///
  /// This is an optional operation that may not be implemented by every
  /// filesystem.
  virtual absl::Status Name(absl::string_view* result) const {
    ignore_result(result);
    return absl::UnimplementedError("This filesystem does not support Name()");
  }

  /// \brief Syncs contents of file to filesystem.
  ///
  /// This waits for confirmation from the filesystem that the contents
  /// of the file have been persisted to the filesystem; if the OS
  /// or machine crashes after a successful Sync, the contents should
  /// be properly saved.
  virtual absl::Status Sync() = 0;

  /// \brief Retrieves the current write position in the file, or -1 on
  /// error.
  ///
  /// This is an optional operation, subclasses may choose to return
  /// errors::Unimplemented.
  virtual absl::Status Tell(int64_t* position) {
    *position = -1;
    return absl::UnimplementedError("This filesystem does not support Tell()");
  }

 private:
  WritableFile(const WritableFile&) = delete;
  WritableFile& operator=(const WritableFile&) = delete;
};

/// \brief A readonly memmapped file abstraction.
///
/// The implementation must guarantee that all memory is accessible when the
/// object exists, independently from the Env that created it.
class ReadOnlyMemoryRegion {
 public:
  ReadOnlyMemoryRegion() {}
  virtual ~ReadOnlyMemoryRegion() = default;

  /// \brief Returns a pointer to the memory region.
  virtual const void* data() = 0;

  /// \brief Returns the length of the memory region in bytes.
  virtual uint64_t length() = 0;
};

/// \brief A registry for file system implementations.
///
/// Filenames are specified as an URI, which is of the form
/// [scheme://]<filename>.
/// File system implementations are registered using the REGISTER_FILE_SYSTEM
/// macro, providing the 'scheme' as the key.
///
/// There are two `Register` methods: one using `Factory` for legacy filesystems
/// (deprecated mechanism of subclassing `FileSystem` and using
/// `REGISTER_FILE_SYSTEM` macro), and one using `std::unique_ptr<FileSystem>`
/// for the new modular approach.
///
/// Note that the new API expects a pointer to `ModularFileSystem` but this is
/// not checked as there should be exactly one caller to the API and doing the
/// check results in a circular dependency between `BUILD` targets.
///
/// Plan is to completely remove the filesystem registration from `Env` and
/// incorporate it into `ModularFileSystem` class (which will be renamed to be
/// the only `FileSystem` class and marked as `final`). But this will happen at
/// a later time, after we convert all filesystems to the new API.
///
/// TODO(mihaimaruseac): After all filesystems are converted, remove old
/// registration and update comment.
class FileSystemRegistry {
 public:
  typedef std::function<FileSystem*()> Factory;

  virtual ~FileSystemRegistry() = default;
  virtual absl::Status Register(const std::string& scheme,
                                      Factory factory) = 0;
  virtual absl::Status Register(
      const std::string& scheme, std::unique_ptr<FileSystem> filesystem) = 0;
  virtual FileSystem* Lookup(const std::string& scheme) = 0;
  virtual absl::Status GetRegisteredFileSystemSchemes(
      std::vector<std::string>* schemes) = 0;
};

/// A utility routine: copy contents of `src` in file system `src_fs`
/// to `target` in file system `target_fs`.
absl::Status FileSystemCopyFile(FileSystem* src_fs, const std::string& src,
                                FileSystem* target_fs, const std::string& target);

}  // namespace gtl

#endif  // GTL_FILE_SYSTEM_H_
