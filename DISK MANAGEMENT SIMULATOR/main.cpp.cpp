#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 256

// ============================================================================
void decToBinary(int n, char &c)
{
    // array to store binary number
    int binaryNum[8];

    // counter for binary array
    int i = 0;
    while (n > 0)
    {
        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }

    // printing binary array in reverse order
    for (int j = i - 1; j >= 0; j--)
    {
        if (binaryNum[j] == 1)
            c = c | 1u << j;
    }
}



// ============================================================================

class FsFile {
    int file_size;
    int block_in_use;
    int index_block;
    int block_size;

public:

    FsFile(int _block_size) {
        file_size = 0;
        block_in_use = 0;
        block_size = _block_size;
        index_block = -1;
    }

    int getfile_size() {
        return file_size;
    }

    void setfile_size(int file_size) {
        this->file_size = file_size;
    }

    int getindex_block() {
        return this->index_block;
    }

    void setindex_block(int index_block) {
        this->index_block = index_block;
    }

    int getblock_in_use() {
        return this->block_in_use;
    }

    void setblock_in_use(int block_in_use) {
        this->block_in_use = block_in_use;
    }

};

// ============================================================================

class FileDescriptor {
    string file_name;
    FsFile* fs_file;
    bool inUse;

public:

    FileDescriptor(string FileName, FsFile* fsi) {
        file_name = FileName;
        fs_file = fsi;
        inUse = true;
    }

    ~FileDescriptor() {
        delete this->fs_file;
    }

    string getFileName() {
        return file_name;
    }

    bool getInUse() {
        return this->inUse;
    }

    void setInUse(bool inUse) {
        this->inUse = inUse;
    }

    FsFile* getFsFile() {
        return this->fs_file;
    }

};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"

// ============================================================================

class fsDisk {
    FILE *sim_disk_fd;
    bool is_formated;

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the
    //             first block is occupied.
    int BitVectorSize;
    int *BitVector;

    // (5) MainDir --
    // Structure that links the file name to its FsFile
    std::vector<FileDescriptor*> MainDir;

    // (6) OpenFileDescriptors --
    //  when you open a file,
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor.
    std::vector<FileDescriptor*> OpenFileDescriptors;

    int blockSize;
    int limitSize;
    int availableBlocks;

#define INVALID_NUMBER (-1)
#define INVALID_STRING ("-1")

public:

    // ------------------------------------------------------------------------
    fsDisk() {
        sim_disk_fd = fopen(DISK_SIM_FILE, "r+");
        assert(sim_disk_fd);

        for (int i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd);
            assert(ret_val == 1);
        }

        fflush(sim_disk_fd);
        is_formated = false;

        this->BitVector = nullptr;
        this->BitVectorSize = 0;

    }

    ~fsDisk() {

        fclose(this->sim_disk_fd);

        delete[] this->BitVector;

        for (FileDescriptor* fileDescriptor : this->MainDir) {

            delete fileDescriptor;

        }

    }

    // ------------------------------------------------------------------------
    void listAll() {

        // loop for presenting all open files

        int i = 0;
        int limit = MainDir.size();
        for (i = 0; i < limit; i++) {

            FileDescriptor* fileDescriptor = this->MainDir[i];
            std::string fileName = fileDescriptor->getFileName();
            bool inUse = fileDescriptor->getInUse();

            std::cout << "index: " << i;
            std::cout << ": FileName: " << fileName;
            std::cout << " , isInUse: " << inUse;
            std::cout << std::endl;

        }

        // loop for presenting the contents of the disk

        char bufy;
        cout << "Disk content: '";
        for (i = 0; i < DISK_SIZE; i++)
        {

            cout << "(";

            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fread(&bufy, 1, 1, sim_disk_fd);

            bool testA = isprint(bufy);
            bool testB = bufy != '\0';

            if (testA) {

                std::cout << bufy;

            }

            if (testB && !testA) {

                std::cout << (int) bufy;

            }

            cout << ")";
        }

        cout << "'" << endl;

    }



    // ------------------------------------------------------------------------
    void fsFormat(int blockSize = 4) {

        // deletes previous bit vector if present
        if (this->BitVector != nullptr) {

            delete[] this->BitVector;

        }

        // initializes members related to the bit vector
        this->blockSize = blockSize;
        this->BitVectorSize = DISK_SIZE / blockSize;
        this->BitVector = new int[this->BitVectorSize];
        this->limitSize = blockSize * blockSize;

        int i = (-1);
        int limit = (-1);

        // loop for initializing all values of the bit vector to zero
        for (i = 0; i < this->BitVectorSize; i++) {

            this->BitVector[i] = 0;

        }

        // loop for deleting all open files
        limit = this->MainDir.size();
        for (i = 0; i < limit; i++) {

            FileDescriptor* fileDescriptor = this->MainDir[i];
            delete fileDescriptor;

        }

        this->MainDir.clear();
        this->OpenFileDescriptors.clear();

        this->availableBlocks = this->BitVectorSize;
        this->is_formated = true;

        std::cout << "FORMAT DISK: number of blocks: " << availableBlocks << std::endl;

    }

    // ------------------------------------------------------------------------
    int CreateFile(string fileName) {

        if (!this->is_formated) {

            return INVALID_NUMBER;

        }

        int i = (-1);
        int j = (-1);
        int limit = (-1);

        // loop for finding an empty space for the newly created file
        limit = this->BitVectorSize;
        bool freeSpace = false;
        for (i = 0; i < limit; i++) {

            if (this->BitVector[i] == 0) {

                freeSpace = true;
                this->BitVector[i] = 1;
                this->availableBlocks--;
                break;

            }

        }

        if (!freeSpace) {

            return INVALID_NUMBER;

        }

        // creates the new file and stores it in MainDir
        auto* fsFile = new FsFile(this->blockSize);
        auto* fileDescriptor = new FileDescriptor(fileName, fsFile);
        this->MainDir.push_back(fileDescriptor);

        int lastIndexDir = this->MainDir.size() - 1;
        FileDescriptor* lastFileDescriptor = this->MainDir[lastIndexDir];
        lastFileDescriptor->getFsFile()->setindex_block(i);

        // searches for an empty space in open descriptors
        limit = OpenFileDescriptors.size();
        for (j = 0; j < limit; j++) {

            FileDescriptor* openFileDescriptor = this->OpenFileDescriptors[j];

            // empty space found, save file on it
            if (openFileDescriptor == nullptr) {

                OpenFileDescriptors[j] = fileDescriptor;

                return j;

            }

        }

        // no empty space found, push descriptor to the end
        this->OpenFileDescriptors.push_back(fileDescriptor);

        int lastIndexOpen = this->OpenFileDescriptors.size() - 1;
        return lastIndexOpen;

    }

    // ------------------------------------------------------------------------
    int OpenFile(string fileName) {

        int i = (-1);
        int limit = (-1);

        FileDescriptor* fileDescriptor = nullptr;
        FileDescriptor* openFileDescriptor = nullptr;

        bool fileFound = false;

        // loop for searching for file with given fileName
        limit = this->MainDir.size();
        for (i = 0; i < limit; i++) {

            fileDescriptor = this->MainDir[i];
            std::string existingFileName = fileDescriptor->getFileName();

            if (existingFileName == fileName) {

                fileFound = true;

                bool inUse = fileDescriptor->getInUse();

                if (inUse) {

                    return INVALID_NUMBER;

                }

                fileDescriptor->setInUse(true);
                break;

            }

        }

        if (!fileFound) {

            return INVALID_NUMBER;

        }

        // loop to search for an empty space
        limit = this->OpenFileDescriptors.size();
        for (i = 0; i < limit; i++) {

            openFileDescriptor = this->OpenFileDescriptors[i];

            // empty space found, save file on it
            if (openFileDescriptor == nullptr) {

                this->OpenFileDescriptors[i] = fileDescriptor;
                return i;

            }

        }

        // no empty space found, push descriptor to the end
        this->OpenFileDescriptors.push_back(fileDescriptor);

        int lastIndexOpen = this->OpenFileDescriptors.size() - 1;
        return lastIndexOpen;

    }

    // ------------------------------------------------------------------------
    string CloseFile(int fd) {

        int lastIndexOpen = this->OpenFileDescriptors.size() - 1;

        // tests for checking if the file number is within bounds and valid
        bool testA = fd >= 0 && fd <= lastIndexOpen;
        bool testB = true;

        if (testA) {

            testB = this->OpenFileDescriptors[fd] == nullptr;

        }

        if (testB) {

            return INVALID_STRING;

        }

        // actions to erase references to the file descriptor
        FileDescriptor* openFileDescriptor = this->OpenFileDescriptors[fd];
        this->OpenFileDescriptors[fd] = nullptr;

        openFileDescriptor->setInUse(false);

        std::string fileName = openFileDescriptor->getFileName();
        return fileName;

    }
    // ------------------------------------------------------------------------
    int WriteToFile(int fd, char *buf, int len) {

        int i = (-1);
        int j = (-1);

        if (!this->is_formated) {

            return INVALID_NUMBER;

        }

        if (fd >= this->OpenFileDescriptors.size()) {

            return INVALID_NUMBER;

        }

        FileDescriptor* openFileDescriptor = this->OpenFileDescriptors[fd];

        if (openFileDescriptor == nullptr) {

            return INVALID_NUMBER;

        }

        FsFile* fsFile = openFileDescriptor->getFsFile();

        int file_size = fsFile->getfile_size();

        if (file_size + len > this->limitSize) {

            return INVALID_NUMBER;

        }

        int offset = file_size % this->blockSize;
        int neededBlock = ((len + offset) - this->blockSize) / this->blockSize;

        if (offset == 0) {

            neededBlock = len / this->blockSize;

        }

        if (neededBlock > this->availableBlocks) {

            return INVALID_NUMBER;

        }

        int index_block = fsFile->getindex_block();

        char binaryDigits[this->blockSize];

        // moves the file cursor to the proper position
        fseek(
                this->sim_disk_fd,
                index_block * this->blockSize,
                SEEK_SET
        );

        // reads data at position
        fread(
                binaryDigits,
                sizeof(char),
                this->blockSize,
                this->sim_disk_fd
        );

        int currentIndex = file_size / this->blockSize;
        int indexBlockFree;

        for (i = 0; i < len; i++) {

            if (offset == 0) {

                bool spaceFound = false;
                for (j = 0; j < this->BitVectorSize; j++) {

                    if (this->BitVector[j] == 0) {

                        spaceFound = true;
                        this->BitVector[j] = 1;
                        this->availableBlocks--;
                        break;

                    }

                }

                if (!spaceFound) {

                    indexBlockFree = -1;
                    return INVALID_NUMBER;

                }

                indexBlockFree = j;

                decToBinary(indexBlockFree, binaryDigits[currentIndex]);

                int block_in_use = fsFile->getblock_in_use() + 1;
                fsFile->setblock_in_use(block_in_use);

            }

            // adjusts file cursor to write position
            fseek(
                    this->sim_disk_fd,
                    binaryDigits[currentIndex] * this->blockSize + offset,
                    SEEK_SET
            );

            // write data at the position
            fwrite(
                    &buf[i],
                    sizeof(char),
                    1,
                    this->sim_disk_fd
            );

            offset++;

            if (offset == this->blockSize) {

                currentIndex++;
                offset = 0;

            }

        }

        fsFile->setfile_size(file_size + len);

        fseek(
                this->sim_disk_fd,
                index_block * this->blockSize,
                SEEK_SET
        );

        fwrite(
                binaryDigits,
                sizeof(char),
                this->blockSize,
                this->sim_disk_fd
        );

        return 0;

    }

    // ------------------------------------------------------------------------
    int DelFile(string FileName) {

        int i = (-1);
        int limit = (-1);

        if (!this->is_formated) {

            return INVALID_NUMBER;

        }

        int fd = INVALID_NUMBER;

        // loop for finding the file number for the given fileName
        limit = this->MainDir.size();
        for (i = 0; i < limit; i++) {

            FileDescriptor* fileDescriptor = this->MainDir[i];
            if (fileDescriptor->getFileName() == FileName) {

                fd = i;
                break;

            }

        }

        if (fd < 0) {

            return INVALID_NUMBER;

        }

        FileDescriptor* fileDescriptor = this->MainDir[fd];
        FsFile* fsFile = fileDescriptor->getFsFile();

        char blockData[this->blockSize];

        int index_block = fsFile->getindex_block();
        int block_in_use = fsFile->getblock_in_use();

        // sets the file cursor at the proper position
        fseek(
                this->sim_disk_fd,
                index_block * this->blockSize,
                SEEK_SET
        );

        // reads data from the position
        fread(
                blockData,
                sizeof(char),
                this->blockSize,
                this->sim_disk_fd
        );

        // at this point, "blockData" holds the indexes allocated for this file

        // block of data for position where data will be erased
        char blockEmpty[this->blockSize];

        // loop for filling block with zeros
        for (int i = 0; i < this->blockSize; i++) {

            blockEmpty[i] = '\0';

        }

        // loop for erasing all indexed blocks held by deleted file
        for (int j = 0; j < block_in_use; j++) {

            fseek(
                    this->sim_disk_fd,
                    blockData[j] * this->blockSize,
                    SEEK_SET
            );

            fwrite(
                    blockEmpty,
                    1,
                    this->blockSize,
                    sim_disk_fd
            );

            this->availableBlocks++;

            BitVector[blockData[j]] = 0;

        }

        // erasing indexes of deleted file
        int position = index_block;

        // sets file cursor to proper position
        fseek(
                sim_disk_fd,
                this->blockSize * position,
                SEEK_SET
        );

        // erases indexes by filling with zeros
        fwrite(
                blockEmpty,
                1,
                this->blockSize,
                this->sim_disk_fd
        );

        this->availableBlocks++;

        this->BitVector[position] = 0;



        return 0;

    }
    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char *buf, int len) {

        if (!this->is_formated) {

            return INVALID_NUMBER;

        }

        if (fd >= this->OpenFileDescriptors.size()) {

            return INVALID_NUMBER;

        }

        int i = (-1);
        int limit = this->OpenFileDescriptors.size();

        bool testA = fd < 0 || fd > limit;
        bool testB = true;

        if (!testA) {

            testB = this->OpenFileDescriptors[fd] == nullptr;

        }

        if (testA || testB) {

            return INVALID_NUMBER;

        }

        FileDescriptor* openFileDescriptor = this->OpenFileDescriptors[fd];
        FsFile* fsFile = openFileDescriptor->getFsFile();
        int file_size = fsFile->getfile_size();
        int index_block = fsFile->getindex_block();

        if (file_size < len) {

            return INVALID_NUMBER;

        }

        int offset = 0, currentIndex = 0;
        char blockData[this->blockSize];
        int position = index_block;

        fseek(
                this->sim_disk_fd,
                position * this->blockSize,
                SEEK_SET
        );

        fread(
                blockData,
                1,
                this->blockSize,
                this->sim_disk_fd
        );

        for (i = 0; i < len; i++) {

            fseek(
                    this->sim_disk_fd,
                    (blockData[currentIndex]* this->blockSize) + offset,
                    SEEK_SET
            );

            fread(
                    &buf[i],
                    1,
                    1,
                    this->sim_disk_fd
            );

            offset++;

            if (offset == this->blockSize) {

                offset = 0;
                currentIndex++;

            }

        }

        buf[len] = '\0';

        return 0;

    }

};

int main() {
    int blockSize = (-1);
    int direct_entries = (-1);
    string fileName;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read = 0;
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while(1) {
        cin >> cmd_;
        switch (cmd_)
        {
            case 0:   // exit
                delete fs;
                exit(0);
                break;

            case 1:  // list-file
                fs->listAll();
                break;

            case 2:    // format
                cin >> blockSize;
                fs->fsFormat(blockSize);
                break;

            case 3:    // creat-file
                cin >> fileName;
                _fd = fs->CreateFile(fileName);
                cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 4:  // open-file
                cin >> fileName;
                _fd = fs->OpenFile(fileName);
                cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 5:  // close-file
                cin >> _fd;
                fileName = fs->CloseFile(_fd);
                cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 6:   // write-file
                cin >> _fd;
                cin >> str_to_write;
                fs->WriteToFile( _fd , str_to_write , strlen(str_to_write) );
                break;

            case 7:    // read-file
                cin >> _fd;
                cin >> size_to_read ;
                fs->ReadFromFile( _fd , str_to_read , size_to_read );
                cout << "ReadFromFile: " << str_to_read << endl;
                break;

            case 8:   // delete file
                cin >> fileName;
                _fd = fs->DelFile(fileName);
                cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;
            default:
                break;
        }

    }

} 
