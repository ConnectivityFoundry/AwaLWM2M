/************************************************************************************************************************
 Copyright (c) 2016, Imagination Technologies Limited and/or its affiliated group companies.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 following conditions are met:
     1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
        following disclaimer.
     2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
        following disclaimer in the documentation and/or other materials provided with the distribution.
     3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
        products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************************************************************************************/

#ifndef FILE_RESOURCE_H
#define FILE_RESOURCE_H

#include <string>
#include <cstdio>
#include <unistd.h>

/**
 * @brief Class to determine a unique temporarily filename.
 */
class TempFilename {
public:
    TempFilename() : filename_(nullptr) {
        filename_ = strdup("/tmp/tmpfileXXXXXX");
        int fd = mkstemp(filename_);
        if (fd != -1)
        {
            close(fd);  // close immediately
        }
        else
        {
            perror("mkstemp");
        }
    }
    ~TempFilename() { free(filename_); filename_ = nullptr; }
    std::string GetFilename() const { return std::string(filename_); }
private:
    char * filename_;
};

/**
 * @brief Intended to be used to create temporary files during tests.
 *        Files are created with a unique, temporary filename, and the content
 *        provided is written to the file. When the object is destroyed, the
 *        file is deleted from the filesystem.
 */
class FileResource {
public:
    explicit FileResource(const char * content) :
      filename_(TempFilename().GetFilename()) {
        // save content to a temporary file
        std::cerr << "Writing content to " << filename_ << std::endl;
        std::ofstream file(filename_, std::ios::out);
        file << content << std::endl;
        file.close();
    }
    ~FileResource() {
        std::cerr << "Deleting " << filename_ << std::endl;
        if (std::remove(filename_.c_str()) < 0)
        {
            std::perror("Error deleting file");
        }
    }
    FileResource(const FileResource & that) = delete;
    FileResource & operator=(const FileResource & that) = delete;


    std::string GetFilename() const {
        std::cerr << "Get filename: " << filename_ << std::endl;
        return filename_;
    }

private:
    std::string filename_;
};

#endif // FILE_RESOURCE_H
