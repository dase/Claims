scp $1 claims@58.198.176.92:~/lizhif/$1 

ssh -f -n -l claims 58.198.176.92 "scp ~/lizhif/$1 10.11.1.191:~/lizhif/"
ssh -f -n -l claims 58.198.176.92 "scp ~/lizhif/$1 10.11.1.193:~/lizhif/"
ssh -f -n -l claims 58.198.176.92 "scp ~/lizhif/$1 10.11.1.194:~/lizhif/"



