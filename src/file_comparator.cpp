#include<iostream>
#include<cstdint>

int compare_files(FILE *fp_v1, FILE *fp_v2){
    size_t bytes_read_v1;
    size_t bytes_read_v2;
    uint8_t buf_v1[BUFSIZ];
    uint8_t buf_v2[BUFSIZ];
    while (true){
        bytes_read_v1 = fread(buf_v1, sizeof(uint8_t), BUFSIZ, fp_v1);
        bytes_read_v2 = fread(buf_v2, sizeof(uint8_t), BUFSIZ, fp_v2);
        if(bytes_read_v1 == bytes_read_v2){
            if(bytes_read_v1){
                for(size_t i=0;i<bytes_read_v1;i++){
                    if(buf_v1[i] != buf_v2[i]){
                        return EXIT_FAILURE;
                    }
                }
            }
            else{
                break;
            }
        }
        else{
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv){
    char *filename_v1 = argv[1];
    char *filename_v2 = argv[2];
    
    FILE *fp_v1 = fopen(filename_v1, "rb");
    FILE *fp_v2 = fopen(filename_v2, "rb");

    if(compare_files(fp_v1, fp_v2)){
        std::cout<<"\nFILES DID NOT MATCH!!\n";
    }
    else{
        std::cout<<"\nFILES PERFECTLY MATCHED!!\n";
    }

    fclose(fp_v1);
    fclose(fp_v2);

    return 0;
}
