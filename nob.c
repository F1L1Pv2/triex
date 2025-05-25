#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32

#define VULKAN_SDK_SEARCH_PATH "C:/VulkanSDK"

const char* vulkanSDKPathLIB;
const char* vulkanSDKPathINC;

#define PLATFORM_COMPILER_ARGS vulkanSDKPathINC,
#define PLATFORM_LINKER_FLAGS vulkanSDKPathLIB, "-lvulkan-1", "-lkernel32", "-luser32", "-lgdi32", "-lshaderc_shared", "-lshaderc_util", "-lglslang", "-lSPIRV", "-lSPIRV-Tools", "-lSPIRV-Tools-opt",
#else
#define PLATFORM_COMPILER_ARGS
#define PLATFORM_LINKER_FLAGS "-lvulkan", "-lX11", "-lXrandr", "-lshaderc",
#endif

#define COMPILER_NAME "clang"
#define OUTPUT_PROGRAM_NAME "main"
#define COMPILER_ARGS PLATFORM_COMPILER_ARGS "-I./", "-I./src"
#define LINKER_FLAGS PLATFORM_LINKER_FLAGS

#ifndef WIN32
int isDirectory(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}
#endif

bool traverse_directory(char* filepath, File_Paths* out){
    File_Paths children = {0};
    read_entire_dir(filepath,&children);
    for(int i = 0; i < children.count; i++){
        if(children.items[i][0] == '.') continue;
        String_Builder sb = {0};
        String_View sv = sv_from_cstr(children.items[i]);
        sv_chop_by_delim(&sv,'.');

        sb_append_cstr(&sb, filepath);
        if(sb.items[sb.count-1] != '/' && sb.items[sb.count-1] != '\\') sb_append_cstr(&sb, "/");
        sb_append_cstr(&sb,children.items[i]);
        sb_append_null(&sb);


#ifdef WIN32
        if(strlen(children.items[i]) == sv.count || sv.count == 0){
#else  
        if(isDirectory(sb.items)) {
#endif
            File_Paths inner_children = {0};
            traverse_directory(sb.items,&inner_children);
            for(int j = 0; j < inner_children.count; j++){
                da_append(out, inner_children.items[j]);
            }
            da_free(inner_children);
        }else{
            da_append(out, sb.items);
        }
    }

    da_free(children);
    return true;
};

String_View chopUntilEnd(String_View in, char ch){
    String_View sv = in;

    while(sv.count != 0 && sv.data - in.data < in.count){
        in = sv;
        sv_chop_by_delim(&sv,ch);
    }
    
    return in;
}

void filter_out_paths_ending(char** ending, size_t ending_count, File_Paths* in){
    File_Paths intermediate = {0};

    for(int i = 0; i < in->count; i++){
        String_View sv = sv_from_cstr(in->items[i]);
        if(sv.count == 0) continue;
        sv = chopUntilEnd(sv,'.');
        for(int j =0; j < ending_count; j++){
            if(strcmp(sv.data,ending[j]) == 0){
                da_append(&intermediate, in->items[i]);
                break;
            }
        }
    }

    da_free(*in);
    in->items = intermediate.items;
    in->capacity = intermediate.capacity;
    in->count = intermediate.count;
}

String_View remove_last_part_from_path(char* filename){
    String_View sv = sv_from_cstr(filename);
    if(sv.count == 0) return sv;

    String_View sv2 = sv_from_cstr(filename);
    
    while (true){
        String_View sv3 = sv2;
        sv_chop_by_delim(&sv3,'/');
        if(sv3.count == 0 || (sv3.data-sv.data) == sv.count) break;
        sv2 = sv3;
    }

    sv.count = sv2.data-sv.data;

    return sv;
}

void make_needed_folders_recursive(String_View filename){
    if(filename.count == 0) return;

    String_View sv = filename;

    String_Builder sb = {0};

    while(sv.count > 0){
        sv_chop_by_delim(&sv,'/');
        sb.count = 0;
        sb_append_buf(&sb,filename.data,sv.data-filename.data);
        sb_append_null(&sb);
        mkdir_if_not_exists(sb.items);
    }

    sb_free(sb);
}

bool change_extension(String_Builder* sb, char* filename, char* new_extension){
    String_View sv = sv_from_cstr(filename);

    sv_chop_by_delim(&sv,'.');
    if(sv.count == 0) return false;
    sb_append_buf(sb,filename, sv.data-filename);
    sb_append_cstr(sb,new_extension);
    return true;
}

#define BUILD_PATH (debug ? "build/debug/" : "build/release/")

const char* strltrim(const char* data) {
    while(data[0] && isspace(data[0])) data++;
    return data;
}

void remove_backslashes(char* data) {
    char* backslash;
    // NOTE: Assumes strchr returns NULL on not found
    while((backslash=strchr(data, '\\'))) {
        switch(backslash[1]) {
        case '\n':
            memmove(backslash, backslash+2, strlen(backslash+2)+1);
            break;
        default:
            memmove(backslash, backslash+1, strlen(backslash+1)+1);
        }
        data=backslash;
    }
}

void fix_backslashes(char* data) {
    char* current_pos = data;
    
    char* cr_pos;
    while ((cr_pos = strchr(current_pos, '\r'))) {
        memmove(cr_pos, cr_pos + 1, strlen(cr_pos + 1) + 1);
        current_pos = cr_pos;
    }

    current_pos = data;
    char* backslash;
    while ((backslash = strchr(current_pos, '\\'))) {
        if (backslash[1] != '\r' && backslash[1] != '\n') {
            *backslash = '/';
        }
        current_pos = backslash + 1;
    }
}

bool dep_analyse_str(char* data, char** result, Nob_File_Paths* paths) {
    // NOTE: Assumes strchr returns NULL on not found
    char* result_end = strchr(data, ':');
    if(!result_end) return false;
    result_end[0] = '\0';
    *result = data;
    data = result_end+1;
    remove_backslashes(data);
    char* lineend;
    if((lineend=strchr(data, '\n')))
        lineend[0] = '\0'; // Ignore all the stuff after the newline
    while((data=(char*)strltrim(data))[0]) {
        char* path=data;
        while(data[0] && data[0] != ' ') data++;
        nob_da_append(paths, path);
        if(data[0]) {
            data[0] = '\0';
            data++;
        }
    }
    return true;
}

#include <sys/stat.h> // for stat()
#include <stdbool.h>  // for bool type

#ifdef _WIN32
#include <windows.h>
#endif

static bool folder_exists(const char* path) {
#ifdef _WIN32
    DWORD attrib = GetFileAttributesA(path);
    return (attrib != INVALID_FILE_ATTRIBUTES && 
           (attrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    if (stat(path, &st) != 0) return false;
    return S_ISDIR(st.st_mode);
#endif
}

bool build(Cmd* cmd, String_Builder* sb, String_Builder* sb2, char* filename, bool debug){
    sb->count = 0;
    sb_append_cstr(sb, BUILD_PATH);
    if(!change_extension(sb,filename, "o")) return false;
    sb_append_null(sb);

    String_View sv = remove_last_part_from_path(sb->items);
    sb2->count = 0;
    sb_append_buf(sb2,sv.data,sv.count);
    sb_append_null(sb2);

    if(!folder_exists(sb2->items)) make_needed_folders_recursive(sv);

    cmd_append(cmd,COMPILER_NAME, "-c", filename, "-o", sb->items, "-MP", "-MMD", COMPILER_ARGS);
    if(!debug) cmd_append(cmd, "-O3");
    else cmd_append(cmd, "-g", "-D","DEBUG");
    return cmd_run_sync_and_reset(cmd);
}

const char* nob_get_ext(const char* path) {
    const char* end = path;
    while(*end) end++;
    while(end >= path) {
        if(*end == '.') return end+1;
        if(*end == '/' || *end == '\\') break;
        end--;
    }
    return path + strlen(path);
}

bool nob_c_needs_rebuild(Nob_String_Builder* string_buffer, Nob_File_Paths* paths, const char* output_path, const char **input_paths, size_t input_paths_count) {
    // Reset before usage
    paths->count = 0;
    string_buffer->count = 0;
    size_t temp = nob_temp_save();
    const char* ext = nob_get_ext(output_path);
    const char* d_file = nob_temp_sprintf("%.*sd", (int)(ext - output_path), output_path);

    if(nob_needs_rebuild(d_file, input_paths, input_paths_count) != 0) {
        nob_temp_rewind(temp);
        return true;
    }

    
    if(!nob_read_entire_file(d_file, string_buffer)) {
        nob_temp_rewind(temp);
        return true;
    }
    nob_da_append(string_buffer, '\0');
    
    char* obj;

    fix_backslashes(string_buffer->items);

    if(!dep_analyse_str(string_buffer->items, &obj, paths)) {
        nob_temp_rewind(temp);
        return true;
    }
    
    NOB_UNUSED(obj);
    bool res = nob_needs_rebuild(output_path, paths->items, paths->count) != 0;
    nob_temp_rewind(temp);
    return res;
}

bool nob_c_needs_rebuild1(Nob_String_Builder* string_buffer, Nob_File_Paths* paths, const char* output_path, const char* input_path) {
    return nob_c_needs_rebuild(string_buffer, paths, output_path, &input_path, 1);
}

bool link_files(Cmd* cmd, char* output_filename, char** paths, size_t paths_count, bool debug){
    cmd_append(cmd, COMPILER_NAME);
    for(int i = 0; i < paths_count; i++){
        String_Builder sb = {0};
        sb_append_cstr(&sb, BUILD_PATH);
        if(!change_extension(&sb,paths[i], "o")) return false;
        sb_append_null(&sb);

        cmd_append(cmd, sb.items);
    }
    cmd_append(cmd, "-o", output_filename, LINKER_FLAGS);
    if(debug) cmd_append(cmd,"-g");

    return cmd_run_sync_and_reset(cmd);
}

void remove_directory(const char *path) {
#ifdef _WIN32
    WIN32_FIND_DATA find_data;
    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", path);

    HANDLE hFind = FindFirstFile(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }

        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s\\%s", path, find_data.cFileName);

        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            remove_directory(full_path);
        } else {
            DeleteFile(full_path);
        }
    } while (FindNextFile(hFind, &find_data));

    FindClose(hFind);
    RemoveDirectory(path);
#else
    DIR *dir = opendir(path);
    if (dir == NULL) {
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (lstat(full_path, &statbuf) == -1) {
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            remove_directory(full_path);
        } else {
            unlink(full_path);
        }
    }
    closedir(dir);
    rmdir(path);
#endif
}

#ifdef _WIN32
bool is_version_newer(const char* v1, const char* v2) {
    char buf1[256];
    char buf2[256];
    strncpy(buf1, v1, sizeof(buf1) - 1);
    strncpy(buf2, v2, sizeof(buf2) - 1);
    buf1[sizeof(buf1) - 1] = '\0';
    buf2[sizeof(buf2) - 1] = '\0';
    char* token1 = strtok(buf1, ".");
    char* token2 = strtok(buf2, ".");
    while (token1 != NULL || token2 != NULL) {
        int num1 = (token1 != NULL) ? atoi(token1) : 0;
        int num2 = (token2 != NULL) ? atoi(token2) : 0;
        if (num1 > num2) return true;
        else if (num1 < num2) return false;
        token1 = strtok(NULL, ".");
        token2 = strtok(NULL, ".");
    }
    return false;
}
#endif

int main(int argc, char** argv){
    NOB_GO_REBUILD_URSELF(argc,argv);

#ifdef _WIN32
    if(folder_exists(VULKAN_SDK_SEARCH_PATH)){
        Nob_File_Paths children = {0};
        read_entire_dir(VULKAN_SDK_SEARCH_PATH,&children);
        if(children.count == 0){
            printf("ERROR: No versions inside VulkanSDK folder %s", VULKAN_SDK_SEARCH_PATH);
            return 1;
        }
        const char* chosenVersion = children.items[0];
        for(int i = 1; i < children.count; i++){
            if(is_version_newer(children.items[i], chosenVersion)){
                chosenVersion = children.items[i];
            }
        }
        vulkanSDKPathINC = temp_sprintf("-I%s/%s/Include",VULKAN_SDK_SEARCH_PATH,chosenVersion);
        vulkanSDKPathLIB = temp_sprintf("-L%s/%s/Lib",VULKAN_SDK_SEARCH_PATH,chosenVersion);
    }else{
        printf("ERROR: No VulkanSDK found on system (tried this path \"%s\")", VULKAN_SDK_SEARCH_PATH);
        return 1;
    }
#endif

    Cmd cmd = {0};
    char* program = shift_args(&argc,&argv);
    
    bool debug = true;
    bool run_after = false;
    bool clean = false;
    
    while(argc > 0){
        char* arg = shift_args(&argc,&argv);
        if(strcmp(arg, "release") == 0){
            debug = false;
            continue;
        }
        if(strcmp(arg,"run") == 0){
            run_after = true;
            continue;
        }
        if(strcmp(arg,"clean") == 0){
            clean = true;
            continue;
        }
    }
    
    if(clean){
        remove_directory("build");
        return 0;
    }
    
    #ifdef _WIN32
    char* outputfilename = debug ? "build/debug/"OUTPUT_PROGRAM_NAME".exe" : "build/release/"OUTPUT_PROGRAM_NAME".exe";
    #else
    char* outputfilename = debug ? "build/debug/"OUTPUT_PROGRAM_NAME : "build/release/"OUTPUT_PROGRAM_NAME;
    #endif
    
    String_Builder sb = {0};
    String_Builder sb2 = {0};
    String_Builder sb3 = {0};
    File_Paths paths = {0};
    File_Paths src_paths = {0};
    
    bool needed_rebuild = false;
    
    if(!traverse_directory("src",&src_paths)) return 1;
    char* ending = "c";
    
    filter_out_paths_ending(&ending,1,&src_paths);
    
    for(int i = 0; i < src_paths.count; i++) {
        sb2.count = 0;
        sb_append_cstr(&sb2,BUILD_PATH);
        change_extension(&sb2,src_paths.items[i],"o");
        sb_append_null(&sb2);
        
        if(!nob_c_needs_rebuild1(&sb, &paths, sb2.items,src_paths.items[i])) continue;
        needed_rebuild = true;
        if(!build(&cmd, &sb, &sb3,(char**)src_paths.items[i], debug)) return 1;
    }

    if(needed_rebuild || !file_exists(outputfilename))
        if(!link_files(&cmd,outputfilename,(char**)src_paths.items, src_paths.count, debug)) return 1;


    if(run_after){
        cmd.count = 0;
        cmd_append(&cmd, outputfilename);
        cmd_run_sync_and_reset(&cmd);
    }

    return 0;
}