//Yousif Hibi 208082081
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <assert.h>
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
    int getfile_size(){
        return file_size;
    }
    void setFile_size(int file_size){
        this->file_size=file_size;
    }
    int getBlock_in_use(){
        return block_in_use;
    }
    void setBlock_in_use(int block_in_use){
        this->block_in_use=block_in_use;
    }
    int getIndex_block(){
        return index_block;
    }
    void setIndex_block(int index_block){
        this->index_block=index_block;
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
    string getFileName() {
        return file_name;
    }
    bool getInUse(){
        return inUse;
    }
    void setInUse(bool inUse){
        this->inUse=inUse;
    }
    FsFile* getFsFile(){
        return fs_file;
    }
    void setFsFile(FsFile* fsFile){
        this->fs_file=fs_file;
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


    FileDescriptor **MainDir ;
    vector <FileDescriptor> OpenFileDescriptors;
// (6) OpenFileDescriptors --
//  when you open a file,
// the operating system creates an entry to represent that file
// This entry number is the file descriptor.
int getEmptyPlaceInDisk() {
        int DiskIndex;
        for (int i = 0; i <= BitVectorSize; ++i) {
            if (i == BitVectorSize) {
                cout<<"the disk is full"<<endl;
                return -1;
            }
            if (BitVector[i] == 0) {
                DiskIndex = i + 48;
                BitVector[i] = 1;
                 break;
            }
        }
        return DiskIndex;
    }
    int howManyRemaning(int fd){
        int placeInDisk =0,charInBlock=0,block_size = DISK_SIZE/ BitVectorSize;
        char readChar[1];
        if (OpenFileDescriptors[fd].getFsFile()->getBlock_in_use()==0){
            placeInDisk= getEmptyPlaceInDisk();
            if(placeInDisk==-1){
                return -1;
            }
            strcpy(readChar,"");
            decToBinary(placeInDisk,readChar[0]);
            fseek(sim_disk_fd, (OpenFileDescriptors[fd].getFsFile()->getIndex_block() * block_size)+OpenFileDescriptors[fd].getFsFile()->getBlock_in_use(), SEEK_SET);
           if( fwrite( readChar,  1 , 1, sim_disk_fd)==-1){
               perror("filed to wright on file");
               exit(1);
           }
            OpenFileDescriptors[fd].getFsFile()->setBlock_in_use(OpenFileDescriptors[fd].getFsFile()->getBlock_in_use()+1);
            return block_size*block_size;
        }else{
            fseek(sim_disk_fd, (OpenFileDescriptors[fd].getFsFile()->getIndex_block() * block_size)+OpenFileDescriptors[fd].getFsFile()->getBlock_in_use()-1, SEEK_SET);
            strcpy(readChar,"");
            fread(readChar, 1, 1, sim_disk_fd);
            placeInDisk = (int)readChar[0]-48;
            fseek(sim_disk_fd,placeInDisk *block_size,SEEK_SET);
            for (int i = 0; i <=block_size; ++i) {
                if(i==block_size&&OpenFileDescriptors[fd].getFsFile()->getBlock_in_use()+1<=block_size){
                    placeInDisk= getEmptyPlaceInDisk();
                    if(placeInDisk==-1){
                        return -1;
                    }
                    strcpy(readChar,"");
                    decToBinary(placeInDisk,readChar[0]);
                    fseek(sim_disk_fd, (OpenFileDescriptors[fd].getFsFile()->getIndex_block() * block_size)+OpenFileDescriptors[fd].getFsFile()->getBlock_in_use(), SEEK_SET);
                   if( fwrite( readChar,  1 , 1, sim_disk_fd)==-1){
                       perror("filed to wright on file");
                       exit(1);

                   }
                    OpenFileDescriptors[fd].getFsFile()->setBlock_in_use(OpenFileDescriptors[fd].getFsFile()->getBlock_in_use()+1);
                    charInBlock=0;
                    break;
                }
                strcpy(readChar,"");
                fseek(sim_disk_fd,placeInDisk*block_size+i,SEEK_SET);
                fread(readChar, 1, 1, sim_disk_fd);
                if(readChar[0]=='\0'){
                    charInBlock=i;
                    break;
                }
            }
            return OpenFileDescriptors[fd].getFsFile()->getfile_size()- OpenFileDescriptors[fd].getFsFile()->getBlock_in_use()*block_size+(block_size-charInBlock);
        }
}
public:

// ------------------------------------------------------------------------
fsDisk() {
    sim_disk_fd = fopen(DISK_SIM_FILE , "r+");
    assert(sim_disk_fd);
    for (int i=0; i < DISK_SIZE ; i++) {
        int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
        ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd);
        assert(ret_val == 1);
    }
    fflush(sim_disk_fd);
    is_formated = false;


}

// ------------------------------------------------------------------------
void listAll() {


    int i = 0;
    for (int i=0;i<OpenFileDescriptors.size();i++ ) {

        cout << "index: " << i << ": FileName: " << OpenFileDescriptors[i].getFileName()  <<  " , isInUse: " << OpenFileDescriptors[i].getInUse() << endl;

    }
    char bufy;
    cout << "Disk content: '";
    for (i = 0; i < DISK_SIZE; i++)
    {
        cout << "(";
        int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
        ret_val = fread(&bufy, 1, 1, sim_disk_fd);
        cout << bufy;
        cout << ")";
    }
    cout << "'" << endl;

    }

// ------------------------------------------------------------------------
void fsFormat( int blockSize =4 ) {
        if(blockSize>(DISK_SIZE/2)){//check if the block size if bigger then half of the disk
            cout<<"the size of the disk cant handle this block size"<<endl;
            return;
        }
        else {
            BitVectorSize = DISK_SIZE / blockSize;//set the number of blocks
            BitVector= new int[BitVectorSize];
            for (int i = 0; i < BitVectorSize; i++) {// put 0's in the bit vector to ensure that all the blocks are 0
                BitVector[i]=0;
            }
            cout<<"FORMAT DISK: number of blocks : "<<BitVectorSize<<endl;// massege to say what size does the disk has
            fsDisk();//format disk
            is_formated = true;//the disk is formatted
           MainDir=new FileDescriptor*[BitVectorSize];// allocat MainDir
           OpenFileDescriptors.clear();// clear the Vector
        }
}

// ------------------------------------------------------------------------
int CreateFile(string fileName) {
    if(is_formated){//check if the file is formatted
    int block_size = DISK_SIZE/ BitVectorSize;//set block size
    FsFile *fs= new FsFile(block_size);// construct a fs file
    fs->setFile_size(block_size*block_size);//set file size
    int place=0;
        for (int i = 0; i <  BitVectorSize; i++)
        {// put in maindir
            if ( MainDir[i]==NULL)
            {//check where is NULL
                MainDir[i]=new FileDescriptor(fileName, fs);//construct MainDir index
                MainDir[i]->setInUse(true);//set in use to true
                place=i;
                break;
            }
            if(MainDir[i]->getFileName()==fileName){
                cout<<"there is a file with the same name "<<endl;
                return -1;
            }
        }
        OpenFileDescriptors.push_back(*MainDir[place]);// but the Main Dir file descriptor in the OpenFileDescriptors vector
         return  OpenFileDescriptors.size()-1;// return what file descriptor
    }
    else{
        cout<<"the Disk has not been formatted yet "<<endl;
        return -1;
    }
        return -1;
}
// ---------------------------------------------------------------------------
int OpenFile(string fileName) {
        if(is_formated) {
            int place;
            bool fileExists = false;
            for (int i = 0; i <= BitVectorSize; i++) {
                if(i==BitVectorSize){
                    cout<<"there is no file with this name"<<endl;
                    return -1;
                }
                if (MainDir[i]==NULL){

                }else{
                 if (MainDir[i]->getFileName() == fileName ) {
                    fileExists = true;
                    place = i;
                    break;
                }
                }
            }
            if (fileExists == false || MainDir[place]->getInUse() == true) {
                cout<<"file is open"<<endl;
                return -1;
            } else {
                MainDir[place]->setInUse(true);
                OpenFileDescriptors.push_back(*MainDir[place]);
                return   OpenFileDescriptors.size()-1;
            }
        }else {cout<<"the Disk has not been formatted yet "<<endl;
return -1;}
        return -1;
}
// ------------------------------------------------------------------------
string CloseFile(int fd) {
if(is_formated){
    if(fd< OpenFileDescriptors.size()) {
        int fdMain = 0;
        for (int i = 0; i <= BitVectorSize; ++i) {
            if(i==BitVectorSize){
                cout<<"there is no file with this fd"<<endl;
                return "-1";
            }
            if (MainDir[i]==NULL){
            }else{
            if (OpenFileDescriptors[fd].getFileName() == MainDir[i]->getFileName()) {
                if (MainDir[i]->getInUse() == false) {
                    cout<<"file not in use"<<endl;
                    return "-1";
                } else {
                    fdMain = i;
                    break;
                }
            }
            }
        }
        MainDir[fdMain]->setInUse(false);
        OpenFileDescriptors.erase(OpenFileDescriptors.begin() + fd);
        return MainDir[fdMain]->getFileName();
    }else{
        cout<<"there is no file with this file disruptor  , not open or not created "<<endl;
        return "-1";
    }
}else{
    cout<<"the Disk has not been formatted yet "<<endl;
        return "-1";
}
}
// ------------------------------------------------------------------------
int WriteToFile(int fd, char *buf, int len ) {
    int placeInDisk =0,charLeft=0;
    int block_size = DISK_SIZE/ BitVectorSize;
    char readChar[1];
if(is_formated&&OpenFileDescriptors[fd].getInUse()&&fd< OpenFileDescriptors.size()){
    if (OpenFileDescriptors[fd].getFsFile()->getIndex_block()==-1) {
        placeInDisk= getEmptyPlaceInDisk();
       if (placeInDisk==-1){
           return -1;
       }
        OpenFileDescriptors[fd].getFsFile()->setIndex_block(placeInDisk-48);
    }
     if (OpenFileDescriptors[fd].getFsFile()->getBlock_in_use()>=0){
         charLeft= howManyRemaning(fd);
        if(charLeft<=0){
            cout<<"file is full"<<endl;
            return -1;
        }
     }
        if(len<=charLeft){
            int j=0;
            fseek(sim_disk_fd, (OpenFileDescriptors[fd].getFsFile()->getIndex_block() * block_size)+OpenFileDescriptors[fd].getFsFile()->getBlock_in_use()-1, SEEK_SET);
            strcpy(readChar,"");
            fread(readChar, 1, 1, sim_disk_fd);
            placeInDisk= (int)readChar[0]-48;
            fseek(sim_disk_fd,placeInDisk*block_size,SEEK_SET);
            int firstFree=0;
            for (int i = 0; i < block_size; ++i) {
                    fseek(sim_disk_fd,placeInDisk*block_size+i,SEEK_SET);
                    fread(readChar, 1, 1, sim_disk_fd);
                    if(readChar[0]=='\0'){
                        firstFree=i;
                        break;
                    }
            }
            for (int i = firstFree; i < block_size; ++i) {
                fseek(sim_disk_fd,placeInDisk*block_size+i,SEEK_SET);
              if(  fwrite(&buf[j],1,1,sim_disk_fd)==-1){
                  perror("filed to wright on file");
                  exit(1);
              }
                len--;
                j++;
            }
            while (len>0){
                placeInDisk= getEmptyPlaceInDisk();
                if(placeInDisk==-1){
                    return -1;
                }
                strcpy(readChar,"");
               decToBinary(placeInDisk,readChar[0]);
               fseek(sim_disk_fd, (OpenFileDescriptors[fd].getFsFile()->getIndex_block() * block_size)+OpenFileDescriptors[fd].getFsFile()->getBlock_in_use(), SEEK_SET);
             if(fwrite( readChar,  1 , 1, sim_disk_fd)==-1){
                 perror("filed to wright on file");
                 exit(1);
             }
                OpenFileDescriptors[fd].getFsFile()->setBlock_in_use(OpenFileDescriptors[fd].getFsFile()->getBlock_in_use()+1);
               fseek(sim_disk_fd, (OpenFileDescriptors[fd].getFsFile()->getIndex_block() * block_size)+OpenFileDescriptors[fd].getFsFile()->getBlock_in_use()-1, SEEK_SET);
                strcpy(readChar,"");
               fread(readChar, 1, 1, sim_disk_fd);
                placeInDisk= (int)readChar[0]-48;
               fseek(sim_disk_fd,placeInDisk*block_size,SEEK_SET);
               for (int i = 0; i < block_size; ++i) {
                   fseek(sim_disk_fd,placeInDisk*block_size+i,SEEK_SET);
                if(   fwrite(&buf[j],1,1,sim_disk_fd)==-1){
                    perror("filed to wright on file");
                    exit(1);
                }
                   len--;
                   j++;
                   if(len==0){
                       return fd;
                   }
               }
           }

        }else{
            cout<<"the string entered is bigger then then the place available"<<endl;
            return -1;
        }
}
else{
        cout<<"the Disk has not been formatted yet "<<endl;
return -1;}
        return -1;
}
// ------------------------------------------------------------------------
int ReadFromFile(int fd, char *buf, int len ) {

     int blockSize= DISK_SIZE/BitVectorSize,count=0,placeInDisk =0;
     char readChar[1],putCharInBuff[1];
if (is_formated && OpenFileDescriptors[fd].getFsFile()->getfile_size()>=len&&OpenFileDescriptors[fd].getFsFile()->getIndex_block()!=-1&&fd< OpenFileDescriptors.size()){
    for (int i = 0; i < blockSize; ++i) {
        strcpy(readChar,"");
        fseek(sim_disk_fd,OpenFileDescriptors[fd].getFsFile()->getIndex_block()*blockSize+i,SEEK_SET);
        fread(readChar, 1, 1, sim_disk_fd);
        if( readChar[0]!='\0'){
            placeInDisk=(int)readChar[0]-48;
        for (int j = 0; j < blockSize; ++j) {
            if(len>0){
             fseek(sim_disk_fd, placeInDisk*blockSize+j,SEEK_SET);
                strcpy(putCharInBuff,"");
                 fread(putCharInBuff, 1, 1, sim_disk_fd);
                if(putCharInBuff[0]!='\0'){
                    strcat(buf,putCharInBuff);
                    len--;
                    if (len==0){
                        return fd;
                    }
                }else{
                    cout<<"cann't reead the number of chars "<<endl;
                    return fd;
                }
            }
        }
    }else{ cout<<"cann't reead the number of chars "<<endl;

            return fd;
        }
    }
    return fd;
}
        cout<<"the Disk has not been formatted yet "<<endl;
     strcpy(buf,"");
    return -1;
}
// ------------------------------------------------------------------------
int DelFile(string FileName){
    int blockSize = DISK_SIZE/BitVectorSize,fd=0,placeInDisk=0;
        char readChar[1],putCharInBuff[1];
    if(is_formated){

        for (int i = 0; i <=BitVectorSize; ++i) {
            if(i==BitVectorSize){
                cout<<"there is no file with this name"<<endl;
                return -1;
            }
            if (MainDir[i]==NULL){

            }else{

            if(MainDir[i]->getFileName()==FileName){
                if(MainDir[i]->getInUse()==0){
                fd=i;
                break;
                }else{
                    cout<<"there is no file descriptor with this name  "<<endl;
                    return -1;
                }
            }
            }
        }
        for (int i = 0; i < MainDir[fd]->getFsFile()->getBlock_in_use(); ++i) {
            fseek(sim_disk_fd,MainDir[fd]->getFsFile()->getIndex_block()*blockSize+i,SEEK_SET);
            strcpy(readChar,"");
            fread(readChar, 1, 1, sim_disk_fd);
            if(readChar[0]!='\0'){
                placeInDisk=(int)readChar[0]-48;
                for (int j = 0; j < blockSize; ++j) {
                        fseek(sim_disk_fd, placeInDisk*blockSize+j,SEEK_SET);
                       strcpy(putCharInBuff,"");
                        fread(putCharInBuff, 1, 1, sim_disk_fd);
                        if(putCharInBuff[0]!='\0'){
                            putCharInBuff[0]='\0';
                            fseek(sim_disk_fd, placeInDisk*blockSize+j,SEEK_SET);
                          if(  fwrite(putCharInBuff,1,1,sim_disk_fd)==-1){
                              perror("filed to wright on file");
                              exit(1);
                          }
                        }
                        else{
                            break;
                        }
                    }
                fseek(sim_disk_fd,MainDir[fd]->getFsFile()->getIndex_block()*blockSize+i,SEEK_SET);
                readChar[0]='\0';
              if(  fwrite(readChar,1,1,sim_disk_fd)==-1){
                  perror("filed to wright on file");
                  exit(1);

              }
                BitVector[placeInDisk]=0;
                }else{
            break;}
            }
                BitVector[MainDir[fd]->getFsFile()->getIndex_block()]=0;
                delete MainDir[fd]->getFsFile();
                MainDir[fd]=NULL;
                return fd;

        }
        cout<<"the Disk has not been formatted yet "<<endl;
        return -1;
    }
};
//////////////////////////////////////////////////////////////////////////////



int main() {
    int blockSize;
    int direct_entries;
    string fileName;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
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
//////////////////////////////////////////////////////
