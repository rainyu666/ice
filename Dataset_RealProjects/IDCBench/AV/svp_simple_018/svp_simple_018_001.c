/*
 * racebench2.1_remarks
 * Filename:svp_simple_018_001
 * Template File:svp_simple_018
 * Created by Beijing Sunwise Information Technology Ltd. on 19/11/25.
 * Copyright © 2019年 Beijing Sunwise Information Technology Ltd. All rights reserved.
 * [说明]:
                                
                                                         
                                 
 *
 *
 *
 *
 */

#include "../common.h"

void svp_simple_018_001_main();

float svp_simple_018_001_func1();

float svp_simple_018_001_func2();

void svp_simple_018_001_isr_func1();

volatile float svp_simple_018_001_para1;
volatile float svp_simple_018_001_para2;

void svp_simple_018_001_main() {
  init();
  float svp_simple_018_001_perimete = 0;
  float svp_simple_018_001_area = 0;
  svp_simple_018_001_perimete = svp_simple_018_001_func1();
  svp_simple_018_001_area = svp_simple_018_001_func2();
}

float svp_simple_018_001_func1() {
  float perimete = 0.0;
  perimete = 2 * svp_simple_018_001_para1 *
             svp_simple_018_001_para2;
  return perimete;
}

float svp_simple_018_001_func2() {
  float area = 0.0;
  area = svp_simple_018_001_para1 *
         svp_simple_018_001_para2 *
         svp_simple_018_001_para2;
  return area;
}

void svp_simple_018_001_isr_func1() {
  svp_simple_018_001_para2 = 1.0;
}

void svp_simple_018_001_isr_1() {
  idlerun();
  svp_simple_018_001_para1 = 2.0;
}

void svp_simple_018_001_isr_2() {
  idlerun();
  svp_simple_018_001_isr_func1();
}
