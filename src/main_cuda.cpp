#include "./headers/util_funcs.h"
#include "./headers/sha3_gpu.cuh"

#define DIGEST_SIZE 64
#define UUID_SIZE 16
#define NUM_UUID_TO_READ 100*1000*1L
#define NUM_UUIDS_PER_BATCH 512*1000*1L
#define COMPACT 1


int main(int argc, char** argv){
    std::string filename;
    if(argc>1) filename = argv[1];
    else ERROR("\nERROR: No filename provided!")

    const size_t num_uuids_to_read = NUM_UUID_TO_READ;
    const size_t num_uuid_per_batch = NUM_UUIDS_PER_BATCH;
    
    const size_t uuid_batch_size_in_bytes = num_uuid_per_batch * UUID_SIZE;
    const size_t digest_size_in_bytes = num_uuid_per_batch * DIGEST_SIZE;
    const size_t num_bytes_to_read = ((UUID_SIZE*2 + 1)*num_uuids_to_read - 1);
    
    int last_dot = filename.find_last_of(".");
    int last_dir = filename.find_last_of("/")+1;
    std::string outfile_name = "./hashed_output/" + filename.substr(last_dir, last_dot-last_dir) + "_hashed_byGPU";
    if(COMPACT) outfile_name += ".bin";
    else outfile_name += ".txt";

    FILE *input_fp = createFilePointer(argv[1]);
    FILE *output_fp = std::fopen(outfile_name.c_str(), "wb+");

    uint8_t *buf = (uint8_t *)malloc(uuid_batch_size_in_bytes);
    uint8_t *output_digest = (uint8_t *)malloc(digest_size_in_bytes);
    // uint8_t *validation_digest = (uint8_t *)malloc(digest_size_in_bytes);
    
    int uuid_char_length = (UUID_SIZE*2 + 1);
    size_t total_raw_bytes_to_batch_read = uuid_char_length*num_uuid_per_batch;
    uint8_t *uuid_raw_vals = (uint8_t *)malloc(total_raw_bytes_to_batch_read);

    int current_batch_num = 0;
    std::size_t total_bytes_read = 0;
    while(true){
        std::cout << "Current batch: " << ++current_batch_num;
        size_t num_uuid_read = getUUID_bytes(input_fp, buf, uuid_raw_vals, UUID_SIZE, total_raw_bytes_to_batch_read, &total_bytes_read);
        std::cout << " | UUIDs read: " << num_uuid_read << std::endl;
        
        uint8_t eof_flag = (total_bytes_read >= num_bytes_to_read) || feof(input_fp);
        
        SHA3_16B_gpu(output_digest, buf, DIGEST_SIZE, UUID_SIZE, num_uuid_read, num_uuid_per_batch);
        // SHA3_16B_cpu(output_digest, buf, DIGEST_SIZE, UUID_SIZE, num_uuid_read);
        // SHA3_16B_validation_v1(output_digest, buf, DIGEST_SIZE, UUID_SIZE, num_uuid_read);
        
        if(COMPACT) writeHashes_compact(output_fp, output_digest, DIGEST_SIZE, num_uuid_read, eof_flag);
        else writeHashes(output_fp, output_digest, DIGEST_SIZE, num_uuid_read, eof_flag);
        
        if(eof_flag) break;
    }

    free(output_digest);
    free(buf);
    free(uuid_raw_vals);
    // free(validation_digest);
    
    fclose(input_fp);
    fclose(output_fp);

    std::cout << "Number of UUIDs to read: "<< num_uuids_to_read
            << "\nTotal bytes read: " << total_bytes_read <<std::endl;

    return 0;
}












































// int main(int argc, char** argv){
//     char *filename;
//     if(argc>1) filename = argv[1];
//     else ERROR("\nERROR: No filename provided!")
    
//     bool encoding_flag = isUTF16(filename);
//     FILE* fp = std::fopen(filename, "rb");
//     if(encoding_flag){
//         uint16_t flag_data[1];
//         int _uv = fread(flag_data, sizeof(uint16_t), 1, fp);
//         std::cerr<<CYAN<< "\n[*] Detected UTF-16 encoding. Discarding initial bytes...\n" <<NORMAL<<std::endl;
//     }

//     uint32_t digest_length = 64;

//     FILE* fp_out = std::fopen("hashed.txt", "w+");

//     uint8_t buf[BUFFER_SIZE];
//     std::size_t total_bytes_read = 0;
//     while(true){
//         std::size_t bytes_read = 0;
//         bytes_read = fread(buf, sizeof(uint8_t), BUFFER_SIZE, fp);
//         if(bytes_read == 0){
//             if(feof(fp)) break;
//             else if(ferror(fp)) ERROR("\nERROR: Unexpected error occurred while reading file!")
//         }
//         else{
//             total_bytes_read += bytes_read;
//             bytes_read -= (bytes_read%16);
//         }
//         // std::cout<<"read data: ";
//         // for(int i=0;i<bytes_read;i++){
//         //     std::cout<<buf[i];
//         // }
//         // std::cout<<"Normal Data: ";
//         // for(int i=0;i<bytes_read;i++){
//         //     std::cout<<buf[i];
//         // }
//         // std::cout<<std::endl;
//         int num_uuid = (bytes_read >> 4);
//         uint8_t *validation_digest = new uint8_t[digest_length*num_uuid];
//         uint8_t *output_digest = new uint8_t[digest_length*num_uuid];
//         uint8_t *output_digest_new = new uint8_t[digest_length*num_uuid];

        
//         SHA3_16B(output_digest_new, buf, digest_length, bytes_read);
//         SHA3_16B_validation_v1(validation_digest, buf, digest_length, bytes_read);
//         SHA3_16B_validation_v2(output_digest, buf, digest_length, bytes_read);

//         std::string d1 = bytesToHexString(validation_digest, digest_length) + "\n";
//         std::string d2 = bytesToHexString(output_digest, digest_length) + "\n";
//         std::string d3 = bytesToHexString(output_digest_new, digest_length) + "\n";
        
//         if(d1==d2 && d2==d3 && d1==d3){
//             // std::cout << "SHA3-512 checksum(GOOOOD): " << d1;
//             // std::cout<<std::endl;
//         }
//         else{
//             std::cout << "SHA3-512 checksum(VALIDN): " << d1;
//             std::cout << "SHA3-512 checksum(OUTPUT): " << d2;
//             std::cout << "SHA3-512 checksum(NEWOUT): " << d3;
//             std::cout<<std::endl;
//             // exit(EXIT_FAILURE);
//         }
//         fwrite(d1.c_str(), sizeof(char), d1.size(), fp_out);
        
//         delete validation_digest;
//         delete output_digest;
//         delete output_digest_new;
//     }
//     std::cout << "Total bytes read: " << total_bytes_read <<std::endl;
//     fclose(fp);
//     fclose(fp_out);

//     return 0;
// }
