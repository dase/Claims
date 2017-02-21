ssh -f -n -l claims 58.198.176.92 "cd lizhif/CLAIMS/build ; make  -j 6 ;"
ssh -f -n -l claims 58.198.176.92 "cd lizhif/CLAIMS/build ; scp claimsserver claims@10.11.1.191:~/lizhif/ ; "
ssh -f -n -l claims 58.198.176.92 "cd lizhif/CLAIMS/build ; scp claimsserver claims@10.11.1.193:~/lizhif/ ; "
ssh -f -n -l claims 58.198.176.92 "cd lizhif/CLAIMS/build ; scp claimsserver claims@10.11.1.194:~/lizhif/ ; "



