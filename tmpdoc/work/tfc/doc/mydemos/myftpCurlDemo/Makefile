all:myupload mydownload

myupload:myuploadexe.c myftplib.c
	gcc -o $@ myuploadexe.c myftplib.c -Llib -lcurl
mydownload:mydownloadexe.c myftplib.c
	gcc -o $@ mydownloadexe.c myftplib.c -Llib -lcurl
