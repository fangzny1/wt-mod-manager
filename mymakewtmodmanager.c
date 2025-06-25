#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <direct.h>

#define MAX_PATH_LENGTH 512
#define MAX_MOD_COUNT 50
#define MAX_NAME_LENGTH 100
#define MOD_FOLDER "mod"
#define DEFAULT_GAME_PATH "D:\\steam\\steamapps\\common\\War Thunder\\sound\\mod"
#define CONFIG_FILE "config.txt"

// mod信息结构体
typedef struct
{
    char name[MAX_NAME_LENGTH];
    char path[MAX_PATH_LENGTH];
} ModInfo;

ModInfo mods[MAX_MOD_COUNT];
int modCount = 0;
char gameSoundPath[MAX_PATH_LENGTH] = DEFAULT_GAME_PATH;

// 函数声明
void scanModFolders();
void displayModList();
void copyModFiles(int modIndex);
void createBackup();
void restoreBackup();
int copyDirectory(const char *source, const char *destination);
int copyFile(const char *source, const char *destination);
void clearScreen();
void waitForEnter();
void createModFolder();
void setGamePath();
void saveConfig();
void loadConfig();

int main()
{
    int choice;

    printf("=== 战争雷霆音效Mod管理器 ===\n\n");

    // 加载配置
    loadConfig();

    // 检查mod文件夹是否存在，不存在则创建
    if (GetFileAttributes(MOD_FOLDER) == INVALID_FILE_ATTRIBUTES)
    {
        printf("未找到mod文件夹，正在创建...\n");
        createModFolder();
    }

    while (1)
    {
        printf("\n=== 主菜单 ===\n");
        printf("1. 扫描可用Mod\n");
        printf("2. 显示Mod列表\n");
        printf("3. 安装Mod\n");
        printf("4. 创建备份\n");
        printf("5. 恢复备份\n");
        printf("6. 设置游戏路径\n");
        printf("7. 显示当前路径\n");
        printf("0. 退出\n");
        printf("请选择操作: ");

        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            scanModFolders();
            break;
        case 2:
            displayModList();
            break;
        case 3:
            if (modCount == 0)
            {
                printf("请先扫描Mod文件夹！\n");
            }
            else
            {
                int modIndex;
                printf("请输入要安装的Mod编号: ");
                scanf("%d", &modIndex);
                if (modIndex >= 1 && modIndex <= modCount)
                {
                    copyModFiles(modIndex - 1);
                }
                else
                {
                    printf("无效的Mod编号！\n");
                }
            }
            break;
        case 4:
            createBackup();
            break;
        case 5:
            restoreBackup();
            break;
        case 6:
            setGamePath();
            break;
        case 7:
            printf("当前游戏路径: %s\n", gameSoundPath);
            break;
        case 0:
            printf("感谢使用！\n");
            return 0;
        default:
            printf("无效选择，请重新输入！\n");
        }

        waitForEnter();
    }

    return 0;
}

// 创建mod文件夹
void createModFolder()
{
    if (CreateDirectory(MOD_FOLDER, NULL))
    {
        printf("mod文件夹创建成功！\n");
        printf("请将您的音效mod文件夹放入 '%s' 目录中\n", MOD_FOLDER);
    }
    else
    {
        printf("创建mod文件夹失败！\n");
    }
}

// 扫描mod文件夹
void scanModFolders()
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    char searchPath[MAX_PATH_LENGTH];

    modCount = 0;

    sprintf(searchPath, "%s\\*", MOD_FOLDER);
    hFind = FindFirstFile(searchPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        printf("无法访问mod文件夹！\n");
        return;
    }

    printf("正在扫描mod文件夹...\n");

    do
    {
        // 跳过 . 和 .. 目录
        if (strcmp(findFileData.cFileName, ".") == 0 ||
            strcmp(findFileData.cFileName, "..") == 0)
        {
            continue;
        }

        // 只处理文件夹
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            strcpy(mods[modCount].name, findFileData.cFileName);
            sprintf(mods[modCount].path, "%s\\%s", MOD_FOLDER, findFileData.cFileName);
            modCount++;

            if (modCount >= MAX_MOD_COUNT)
            {
                printf("警告：mod数量超过最大限制！\n");
                break;
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);

    printf("扫描完成！找到 %d 个mod包\n", modCount);
}

// 显示mod列表
void displayModList()
{
    if (modCount == 0)
    {
        printf("没有找到任何mod包，请先扫描！\n");
        return;
    }

    printf("\n=== 可用Mod列表 ===\n");
    for (int i = 0; i < modCount; i++)
    {
        printf("%d. %s\n", i + 1, mods[i].name);
    }
}

// 复制mod文件
void copyModFiles(int modIndex)
{
    printf("正在安装mod: %s\n", mods[modIndex].name);

    // 检查游戏目录是否存在
    if (GetFileAttributes(gameSoundPath) == INVALID_FILE_ATTRIBUTES)
    {
        printf("错误：找不到游戏音效目录！\n");
        printf("请确认游戏安装路径是否正确: %s\n", gameSoundPath);
        return;
    }

    if (copyDirectory(mods[modIndex].path, gameSoundPath))
    {
        printf("Mod安装成功！\n");
    }
    else
    {
        printf("Mod安装失败！\n");
    }
}

// 递归复制目录
int copyDirectory(const char *source, const char *destination)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    char searchPath[MAX_PATH_LENGTH];
    char sourcePath[MAX_PATH_LENGTH];
    char destPath[MAX_PATH_LENGTH];

    // 创建目标目录
    CreateDirectory(destination, NULL);

    sprintf(searchPath, "%s\\*", source);
    hFind = FindFirstFile(searchPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    do
    {
        if (strcmp(findFileData.cFileName, ".") == 0 ||
            strcmp(findFileData.cFileName, "..") == 0)
        {
            continue;
        }

        sprintf(sourcePath, "%s\\%s", source, findFileData.cFileName);
        sprintf(destPath, "%s\\%s", destination, findFileData.cFileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // 递归复制子目录
            copyDirectory(sourcePath, destPath);
        }
        else
        {
            // 复制文件
            if (copyFile(sourcePath, destPath))
            {
                printf("复制: %s\n", findFileData.cFileName);
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return 1;
}

// 复制单个文件
int copyFile(const char *source, const char *destination)
{
    return CopyFile(source, destination, FALSE);
}

// 创建备份
void createBackup()
{
    char backupPath[MAX_PATH_LENGTH];
    sprintf(backupPath, "backup_sound_mod");

    printf("正在创建备份...\n");

    if (GetFileAttributes(gameSoundPath) != INVALID_FILE_ATTRIBUTES)
    {
        if (copyDirectory(gameSoundPath, backupPath))
        {
            printf("备份创建成功！\n");
        }
        else
        {
            printf("备份创建失败！\n");
        }
    }
    else
    {
        printf("游戏音效目录不存在，无法创建备份！\n");
    }
}

// 恢复备份
void restoreBackup()
{
    char backupPath[MAX_PATH_LENGTH];
    sprintf(backupPath, "backup_sound_mod");

    if (GetFileAttributes(backupPath) == INVALID_FILE_ATTRIBUTES)
    {
        printf("未找到备份文件！\n");
        return;
    }

    printf("正在恢复备份...\n");

    if (copyDirectory(backupPath, gameSoundPath))
    {
        printf("备份恢复成功！\n");
    }
    else
    {
        printf("备份恢复失败！\n");
    }
}

// 清屏
void clearScreen()
{
    system("cls");
}

// 设置游戏路径
void setGamePath()
{
    printf("当前游戏路径: %s\n", gameSoundPath);
    printf("请输入新的游戏路径: ");
    scanf(" %[^\n]", gameSoundPath);

    // 检查路径是否有效
    if (GetFileAttributes(gameSoundPath) == INVALID_FILE_ATTRIBUTES)
    {
        printf("警告：路径不存在或无法访问！\n");
        printf("是否仍要保存此路径？(y/n): ");
        char choice;
        scanf(" %c", &choice);
        if (choice != 'y' && choice != 'Y')
        {
            strcpy(gameSoundPath, DEFAULT_GAME_PATH);
            printf("已恢复为默认路径\n");
            return;
        }
    }

    saveConfig();
    printf("游戏路径设置成功！\n");
}

// 保存配置
void saveConfig()
{
    FILE *file = fopen(CONFIG_FILE, "w");
    if (file != NULL)
    {
        fprintf(file, "%s\n", gameSoundPath);
        fclose(file);
    }
}

// 加载配置
void loadConfig()
{
    FILE *file = fopen(CONFIG_FILE, "r");
    if (file != NULL)
    {
        if (fgets(gameSoundPath, MAX_PATH_LENGTH, file) != NULL)
        {
            // 移除换行符
            size_t len = strlen(gameSoundPath);
            if (len > 0 && gameSoundPath[len - 1] == '\n')
            {
                gameSoundPath[len - 1] = '\0';
            }
        }
        fclose(file);
    }
}

// 等待用户按回车
void waitForEnter()
{
    printf("\n按回车键继续...");
    while (getchar() != '\n')
        ;
    getchar();
}