#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void start() {
    printf("CPU Cycles ===>\t1\t2\t3\t4\t5\t6\t7\t8\t9\n");
    return;
}

void printer(int cycle_num, char mips[5][128], char stages[7][4], int line, int** pipeline) {
    int z;
    int y;
    for (y = 0; y < line; y++) {
        printf("%s",mips[y]);
        for (z = 0; z < 9; z++) {
            if (z != 8) {
                printf("%s\t",stages[pipeline[y][z]]);
            }
            else {
                printf("%s\n",stages[pipeline[y][z]]);
            }
        }
    }
    return;
}

void remake(int line, int** pipeline, int haz_line, int cycle) {
    int** temp_pipe;
    int y;
    int z;
    int temp_line = line - 1;
    temp_pipe = calloc(line, sizeof(int*));
    for (y = 0; y < line; y++) {
        temp_pipe[y] = calloc(9,sizeof(int));
    } 
    for (y = 0; y < line; y++) {
        if (y == haz_line) continue;
        for (z = 0; z < 9; z++) {
            temp_pipe[y][z] = pipeline[y][z];
        }
    }

    for (y = 0; y < temp_line; y++) {
        free(pipeline[y]);
    }
    free(pipeline);
    pipeline = temp_pipe;
}

int line_finish(int* segment) {
    int a;
    for (a = 0; a < 9; a++) {
        if (segment[a] == 5) {
            return 1;
        }
    }
    return 0;
}


void update(int line, int** pipeline, int cycle,int hazard, int haz_line) {
    int x;
    if (hazard == 0) {
        for (x = 0; x <= cycle; x++) {
            if (line == x) break;
            if (line_finish(pipeline[x]) == 1) {
                continue;
            }
            else {
                pipeline[x][cycle] = pipeline[x][cycle-1] + 1;
            }
        }
    }
    else {
        remake(line,pipeline,haz_line,cycle);
        for (x = 0; x < 9; x++) {
            pipeline[haz_line][x] = pipeline[haz_line+1][x];
        }
        

    }
    return;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Invalid number of arguments\n");
        return 1; 
    }

    FILE * fp;
    fp = fopen(argv[1], "r");

    if (!fp) {
        fprintf(stderr, "Error: Cannot open file!\n");
        return 1;
    } 

    char mips_code[5][128];
    char first[5][2], second[5][2], third[5][2];
    int line = 0,i,j,k,cycle_num = 0, haz_line = 0, reg_num;
    int haz1[5], haz2[5];

    for (i = 0; i < 5; i++) {
        haz1[i] = -1;
        haz2[i] = -1;
    }

    while(fgets(mips_code[line],128,fp)) {
        reg_num = 0;
        i = 0;
        while (mips_code[line][i] != '\n') {
            if (mips_code[line][i] == '$') {
                if (reg_num == 0) {
                    first[line][0] = mips_code[line][i+1];
                    first[line][1] = mips_code[line][i+2];
                } 
                else if (reg_num == 1) {
                    second[line][0] = mips_code[line][i+1];
                    second[line][1] = mips_code[line][i+2];
                    if (line > 0) {
                        for (j = 0; j < line; j++) {
                            if (first[j][0] == second[line][0] && first[j][1] == second[line][1]) {
                                haz1[line] = j;
                                break;
                            }
                        }
                    }
                }
                else if (reg_num == 2) {
                    third[line][0] = mips_code[line][i+1];
                    third[line][1] = mips_code[line][i+2];
                    if (line > 0) {
                        for (j = 0; j < line; j++) {
                            if (first[j][0] == third[line][0] && first[j][1] == third[line][1]) {
                                haz2[line] = j;
                                break;
                            }
                        }
                    }
                }
                reg_num++;
                i += 3;
                if (mips_code[line][i] == '\n') break;
            }
            i++;
        }
        mips_code[line][i] = '\t'; //gets rid of new line
        line++;
    }
    int** pipeline;  // 0 is . , 1 is IF, 2 is ID, 3 is EX, 4 is MEM, 5 is WB and 6 is *
    pipeline = calloc(line, sizeof(int*));
    for (i = 0; i < line; i++) {
        pipeline[i] = calloc(9,sizeof(int));
    }
    char stages[7][4] = {".","IF","ID","EX","MEM","WB","*"};

    printf("START OF SIMULATION\n\n");

    while (cycle_num < 9) {
        i = 0; 
        j = 1;
        start();

        if (cycle_num == 0) { //first cycle
            pipeline[0][0] = 1;
            printer(cycle_num, mips_code, stages, line, pipeline);
        }
        else if (cycle_num == 1) { //second cycle
            update(line,pipeline,cycle_num,0,haz_line);
            printer(cycle_num, mips_code, stages, line, pipeline);
        }
        else if (cycle_num == 2) { //third cycle
            update (line,pipeline,cycle_num,0,haz_line);
            printer(cycle_num, mips_code, stages, line, pipeline);
        }
        else if (cycle_num == 3) { //fourth cycle
            if (haz1[1] == -1 && haz2[1] == -1) {
                update(line,pipeline,cycle_num,0,haz_line);
                printer(cycle_num, mips_code, stages, line, pipeline);
            }
            else {
                haz_line++;
                for (k = line; k > haz_line; k--) {
                    strncpy(mips_code[k], mips_code[k-1], sizeof(mips_code[k]));
                }
                strncpy(mips_code[haz_line], "nop\t\t", sizeof(mips_code[haz_line]));
                line++;
                update(line,pipeline,cycle_num,1,haz_line);
                printer(cycle_num, mips_code, stages, line, pipeline);
            }

        }
        else if (cycle_num == 4) { //fifth cycle
            if (haz1[2] == -1 && haz2[2] == -1) {
                update(line,pipeline,cycle_num,i,haz_line);
                printer(cycle_num, mips_code, stages, line, pipeline);
            }
        }
        else if (cycle_num == 5) { //sixth cycle
            if (haz1[3] == -1 && haz2[3] == -1) {
                update(line,pipeline,cycle_num,i,haz_line);
                printer(cycle_num, mips_code, stages, line, pipeline);
            }
        }
        else if (cycle_num == 6) { //seventh cycle
            if (haz1[4] == -1 && haz2[4] == -1) {
                update(line,pipeline,cycle_num,i, haz_line);
                printer(cycle_num, mips_code, stages, line, pipeline);
            }
        }
        else if (cycle_num == 7) { //eighth cycle

        }
        else if (cycle_num == 8) { //ninth cycle

        }


        cycle_num++;
        printf("\n");
        if (cycle_num == line+4) break;
    }

    printf("END OF SIMULATION\n");

    fclose(fp);
    return 0;
}