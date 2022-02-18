#include<iostream>

using namespace std;

const int NPOINTS = 32;

int main()
{
    int original[NPOINTS]; 
    unsigned char *buffer;
    short *result;
    short *tmp;
    const int BUFFER_SIZE = NPOINTS*4;

    for(int i=0;i<NPOINTS;++i)
    {
        original[i]=i*0x4000;
        cout << hex << original[i] << endl;
    }

    buffer=new unsigned char[BUFFER_SIZE];
    memcpy(buffer,original,BUFFER_SIZE);
    cout << "Result:" << endl;

    result=new short[NPOINTS];
    tmp=(short*)(buffer);
    for(int i=0;i<NPOINTS;++i)
    {
        tmp++; 
        result[i]=*tmp;
        tmp++;
        cout << hex << result[i] << endl;
    }
    
    delete[] result;
    delete[] buffer;

    return 0;
}
