k=10
e=2
#skip="--is 0 1   4 5 6 7 8 9  "
skip="--is 0 1 2  4 5 6 7 8 9 "
db_type=0
data=as
exec_path="../build/Vend"
data_path="../resource/${data}-shuf.txt"
path_prefix="../${data}/${k}"
db_path="${path_prefix}/db-shuf"
vend_prefix="${path_prefix}/encode/"
output_path_prefix="${path_prefix}/result"
output_path="${output_path_prefix}/result.csv"
pair_path_prefix="${path_prefix}/pairs/"

echo -e "complete file structure\n"
mkdir -p ${db_path} ${vend_prefix} ${pair_path_prefix} ${output_path_prefix}
echo -e "complete file structure successfully\n"


declare -A exper
exper=([0]="--ib" [1]="--ii" [2]="--id" [3]="--iwq" [4]="--irq" [5]="--ifs" [6]="--irs" [7]="--iws" [8]="--it" [9]="--im")




echo -e "execute\n"
${exec_path} ${skip} -d ${data_path} -t ${db_type} -b ${db_path} -v ${vend_prefix} -o ${output_path} -p ${pair_path_prefix} ${exper[$e]}=ON


echo -e "${exec_path} ${skip} -d ${data_path}  -t ${db_type} -b ${db_path} -v ${vend_prefix} -o ${output_path} -p ${pair_path_prefix} ${exper[$e]}=ON"
echo -e "executed successfully \n"
