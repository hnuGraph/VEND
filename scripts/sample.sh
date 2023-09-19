
e=3
skip="--is 0 1  3  4       9 10  12 13 14 15 16 17   "
db_type=0
data=as
vertex=1696415
degree=13
exec_path="../build/Vend"
data_path="../resource/${data}-shuf.txt"


declare -A exper
exper=([0]="--ib" [1]="--ii" [2]="--id" [3]="--iwq" [4]="--irq" [5]="--ifs" [6]="--irs" [7]="--iws" [8]="--it" [9]="--im" )


for k in {2,4,8,16,32}
do   
    echo -e "***********************k = $k *****************\n"
    path_prefix="../tkde-test/${data}/${k}"
    path_prefix="../${data}/${k}"
    db_path="../${data}/6/db"
    vend_prefix="${path_prefix}/encode_test/"
    output_path_prefix="${path_prefix}/result"
    output_path="${output_path_prefix}/result.csv"
    pair_path_prefix="${path_prefix}/pairs/"
    mkdir -p ${vend_prefix}  ${db_path} ${output_path_prefix} ${pair_path_prefix}
    ${exec_path} ${skip} -d ${data_path} -t ${db_type} -b ${db_path} -v ${vend_prefix} -o ${output_path} -p ${pair_path_prefix} -k ${k} --vs ${vertex} --ds ${degree} ${exper[$e]}=ON
done
