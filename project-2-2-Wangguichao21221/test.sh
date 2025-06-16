#!/bin/bash

rm -f testing/circ_files/proj_2_2_top.circ
rm -f testing/student_output/*

cp proj_2_2_top.circ testing/circ_files/

cd testing
python3 local_grader.py
cd ..
