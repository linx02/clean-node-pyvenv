#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAXFNAME 1000
#define RESET "\033[0m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define CYAN "\033[36m"

typedef enum folder_type {
    NODE_MODULES = 0,
    PY_VENV = 1
} folder_type;

typedef struct folder {
    folder_type folder_type;
    char path[MAXFNAME];
} folder;

void traverse(const char path_name[MAXFNAME]);
void ask_and_delete(const folder *folder_);
void delete_folder(const folder *folder_);
int dump_pyvenv(const folder *folder_);

int main(int argc, char *argv[]) {

    if (argc < 2 || argc > 2 || strlen(argv[1]) == 0) {
        printf("Usage: clean <directory>\n");
        return 1;
    }

    char *root_dir = argv[1];

    DIR *dir = opendir(root_dir);
    if (dir == NULL){
        printf(RED "No matching directory found" RESET "\n");
        return 1;
    }

    closedir(dir);

    printf(CYAN "Cleaning from directory: %s" RESET "\n", root_dir);

    traverse(root_dir);

    return 0;
}

void traverse(const char path_name[MAXFNAME]){

    // Open dir
    DIR *dir = opendir(path_name);

    if (dir != NULL){
        struct dirent *entry;

        while((entry = readdir(dir)) != NULL){

            if (entry->d_type == DT_DIR && strcmp(entry->d_name, "node_modules") == 0){
                char new_path[MAXFNAME];
                snprintf(new_path, sizeof(new_path), "%s/%s", path_name, entry->d_name);

                folder folder_;
                folder_.folder_type = NODE_MODULES;
                strcpy(folder_.path, new_path);

                ask_and_delete(&folder_);

            } else if (entry->d_type == DT_REG && strcmp(entry->d_name, "pyvenv.cfg") == 0) {

                folder folder_;
                folder_.folder_type = PY_VENV;
                strcpy(folder_.path, path_name);

                ask_and_delete(&folder_);
                break;
            } else if (entry->d_type == DT_DIR) {
                // Prevent traversing . & .. which will cause an infinite loop
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
                    continue;
                }

                // Construct new path
                char new_path[MAXFNAME];
                snprintf(new_path, sizeof(new_path), "%s/%s", path_name, entry->d_name);

                traverse(new_path);
            }
            
        }
    }

    // Close dir
    closedir(dir);
};

void ask_and_delete(const folder *folder_) {
    char buffer[10];

    if (folder_->folder_type == PY_VENV){
        printf(YELLOW "pyvenv found at: %s. Delete? (y/n) " RESET, folder_->path);
        fgets(buffer, sizeof(buffer), stdin);
    } else if (folder_->folder_type == NODE_MODULES) {
        printf(YELLOW "node_modules found at: %s. Delete? (y/n) " RESET, folder_->path);
        fgets(buffer, sizeof(buffer), stdin);
    }

    char confirm = buffer[0];
    if (confirm == 'y') {
        int status = 0;
        if (folder_->folder_type == PY_VENV) {
            status = dump_pyvenv(folder_);
        }
        if (status == 0){
            delete_folder(folder_);
        } else {
            printf(YELLOW "Skipped deletion of: %s" RESET "\n", folder_->path);
        }
    }
};

int dump_pyvenv(const folder *folder_) {
    char dump_path[MAXFNAME];

    strcpy(dump_path, folder_->path);

    char *last_slash = strrchr(dump_path, '/');
    if (last_slash != NULL) {
        *last_slash = '\0';
    } else {
        strcpy(dump_path, ".");
    }

    strcat(dump_path, "/venv_dump.txt");
    printf(CYAN "Dumping %s -> %s" RESET "\n", folder_->path, dump_path);

    char command[1024];

    snprintf(command, sizeof(command), "%s/bin/pip freeze > %s", folder_->path, dump_path);

    int result = system(command);

    if (result == 0){
        printf(GREEN "Dumped successfully" RESET "\n");
    } else {
        printf(RED "Failed to dump venv: %s" RESET "\n", folder_->path);
        // Clean up venv_dump.txt
        snprintf(command, sizeof(command), "rm %s", dump_path);
        int result_ = system(command);

        if (result_ != 0){
            printf(RED "Failed to remove venv_dump.txt" RESET "\n");
        }
    }

    return result;
}

void delete_folder(const folder *folder_) {
    char command[1024];

    snprintf(command, sizeof(command), "rm -rf '%s'", folder_->path);
    int result = system(command);
    if (result == 0){
        printf(GREEN "Deleted: %s" RESET "\n", folder_->path);
    } else {
        printf(RED "Failed to delete folder: %s" RESET "\n", folder_->path);
    }
}