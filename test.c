#include <stdio.h>
#include "include/filesystem.h"
#include "include/constantes_lseek.h"


int main() {
	int ret, i, fd, version;
	char buffer[50];
	char buffer1[500000];

	printf("TEST mkFS\n");
	
	// AÑADIDO
	ret = mkFS(0);
	if(ret != 0) {
		printf("TEST mkFS FAILED\n");
		return -1;
	}
	// AÑADIDO
	ret = mkFS(50);
	if(ret != 0) {
		printf("TEST mkFS FAILED\n");
		return -1;
	}
	// AÑADIDO
	ret = mkFS(100);
	if(ret == 0) {
		printf("TEST mkFS FAILED\n");
		return -1;
	}

	ret = mkFS(20);
	if(ret != 0) {
		printf("TEST mkFS FAILED\n");
		return -1;
	}

	printf("TEST mkFS SUCCESS\n");
	printf("TEST mountFS + openFS\n");

	ret = mountFS();
	if(ret != 0) {
		printf("TEST mountFS + openFS FAILED at mountFS\n");
		return -1;
	}

	ret = openFS("test1");
	if(ret == -1) {
		printf("TEST mountFS + openFS FAILED at openFS\n");
		return -1;
	}

	// AÑADIDO
	int aux = openFS("test1");
	if(ret != aux) {
		printf("TEST mountFS + openFS FAILED at openFS\n");
		return -1;
	}
	// AÑADIDO
	fd = ret;
	ret = closeFS(fd);
	if(ret != 0) {
		printf("TEST closeFS + umountFS FAILED at closeFS\n");
		return -1;
	}
	// AÑADIDO
	ret = openFS("test1");
	if(ret == -1) {
		printf("TEST mountFS + openFS FAILED at openFS\n");
		return -1;
	}

	printf("TEST mountFS + openFS SUCCESS\n");
	printf("TEST writeFS + readFS + lseekFS\n");

	for(i = 0; i < 50; i++){
		buffer[i] = i;
	}
	
	ret = writeFS(fd, buffer, 50, &version);
	if(ret != 50 || version != 2) {
		printf("TEST writeFS + readFS + lseekFS FAILED at writeFS\n");
		return -1;
	}

	// AÑADIDOS
	for(i = 0; i < 500000; i++){
		buffer1[i] = i;
	}
	// AÑADIDOS
	ret = writeFS(fd, buffer1, 50, &version);
	if(ret != 50 || version != 3) {
		printf("TEST writeFS + readFS + lseekFS FAILED at writeFS\n");
		return -1;
	}

	// AÑADIDOS
	ret = writeFS(fd, buffer1, 500000, &version);
	if(ret != -1) {
		printf("TEST writeFS + readFS + lseekFS FAILED at writeFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = writeFS(fd, buffer1, 50, &version);
	if(ret != 50 || version != 4) {
		printf("TEST writeFS + readFS + lseekFS FAILED at writeFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = writeFS(fd, buffer1, 50, &version);
	if(ret != 50 || version != 5) {
		printf("TEST writeFS + readFS + lseekFS FAILED at writeFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = writeFS(fd, buffer1, 50, &version);
	if(ret != 50 || version != 6) {
		printf("TEST writeFS + readFS + lseekFS FAILED at writeFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = writeFS(fd, buffer1, 50, &version);
	if(ret != 50 || version != 7) {
		printf("TEST writeFS + readFS + lseekFS FAILED at writeFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = writeFS(fd, buffer1, 50, &version);
	if(ret != 50 || version != 8) {
		printf("TEST writeFS + readFS + lseekFS FAILED at writeFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = writeFS(fd, buffer1, 50, &version);
	if(ret != 50 || version != 9) {
		printf("TEST writeFS + readFS + lseekFS FAILED at writeFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = writeFS(fd, buffer1, 50, &version);
	if(ret != 50 || version != 10) {
		printf("TEST writeFS + readFS + lseekFS FAILED at writeFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = writeFS(fd, buffer1, 50, &version);
	if(ret != -1) {
		printf("TEST writeFS + readFS + lseekFS FAILED at writeFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = closeFS(fd);
	if(ret != 0) {
		printf("TEST closeFS + umountFS FAILED at closeFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = switchFS("test1", 1);
	if(ret != 0) {
		printf("TEST deleteFS + switchFS FAILED at switchFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = deleteFS("test1", 1);
	if(ret != 10) {
		printf("TEST deleteFS + switchFS FAILED at deleteFS--\n");
		return -1;
	}
	// AÑADIDOS
	ret = deleteFS("test1", 10);
	if(ret != 9) {
		printf("TEST deleteFS + switchFS FAILED at deleteFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = deleteFS("test1", 9);
	if(ret != 8) {
		printf("TEST deleteFS + switchFS FAILED at deleteFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = deleteFS("test1", 8);
	if(ret != 7) {
		printf("TEST deleteFS + switchFS FAILED at deleteFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = deleteFS("test1", 7);
	if(ret != 6) {
		printf("TEST deleteFS + switchFS FAILED at deleteFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = deleteFS("test1", 6);
	if(ret != 5) {
		printf("TEST deleteFS + switchFS FAILED at deleteFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = deleteFS("test1", 5);
	if(ret != 4) {
		printf("TEST deleteFS + switchFS FAILED at deleteFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = deleteFS("test1", 4);
	if(ret != 3) {
		printf("TEST deleteFS + switchFS FAILED at deleteFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = deleteFS("test1", 3);
	if(ret != 2) {
		printf("TEST deleteFS + switchFS FAILED at deleteFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = openFS("test1");
	if(ret == -1) {
		printf("TEST mountFS + openFS FAILED at openFS\n");
		return -1;
	}

	for(i = 0; i < 50; i++){
		buffer[i] = i*2;
	}
	ret = writeFS(fd, buffer, 50, &version);
	if(ret != 50 || version != 3) {
		printf("TEST writeFS + readFS + lseekFS FAILED at writeFS--\n");
		return -1;
	}
	ret = lseekFS(fd, 0, SF_SEEK_SET);
	if(ret != 0) {
		printf("TEST writeFS + readFS + lseekFS FAILED at lseekFS\n");
		return -1;
	}
	ret = lseekFS(fd, 160, SF_SEEK_SET);
	if(ret != -1){
		printf("TEST writeFS + readFS + lseekFS FAILED at lseekFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = lseekFS(fd, 300, SF_SEEK_END);
	if(ret == -1){
		printf("TEST writeFS + readFS + lseekFS FAILED at lseekFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = lseekFS(fd, 0, SF_SEEK_SET);
	if(ret != 0) {
		printf("TEST writeFS + readFS + lseekFS FAILED at lseekFS\n");
		return -1;
	}
	ret = readFS(fd, buffer, 50, &version);
	if(ret != 50 || version != 3) {
		printf("TEST writeFS + readFS + lseekFS FAILED at readFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = readFS(fd, buffer, 500, &version);
	if(ret != -1) {
		printf("TEST writeFS + readFS + lseekFS FAILED at readFS\n");
		return -1;
	}

	for(i = 0; i < 50; i++){
		if(buffer[i] != i*2) {
			printf("TEST writeFS + readFS + lseekFS FAILED at readFS\n");
			return -1;
		}
	}

	printf("TEST writeFS + readFS + lseekFS SUCCESS\n");
	printf("TEST deleteFS + switchFS\n");

	ret = closeFS(fd);
	if(ret != 0) {
		printf("TEST closeFS + umountFS FAILED at closeFS\n");
		return -1;
	}
	// AÑADIDOS
	ret = switchFS("test1", 32);
	if(ret != -1) {
		printf("TEST deleteFS + switchFS FAILED at switchFS\n");
		return -1;
	}

	ret = deleteFS("test1", version);
	if(ret != 2) {
		printf("TEST deleteFS + switchFS FAILED at deleteFS\n");
		return -1;
	}
	
	ret = switchFS("test1", 1);
	if(ret != -1) {
		printf("TEST deleteFS + switchFS FAILED at switchFS\n");
		return -1;
	}

	ret = openFS("test1");
	if(ret == -1) {
		printf("TEST mountFS + openFS FAILED at openFS\n");
		return -1;
	}
	ret = lseekFS(fd, 40, SF_SEEK_SET);
	if(ret != 40){
		printf("TEST deleteFS + switchFS FAILED at switchFS\n");
		return -1;
	}

	printf("TEST deleteFS + switchFS SUCCESS\n");
	printf("TEST closeFS + umountFS\n");

	ret = closeFS(fd);
	if(ret != 0) {
		printf("TEST closeFS + umountFS FAILED at closeFS\n");
		return -1;
	}
	ret = umountFS();
	if(ret != 0) {
		printf("TEST closeFS + umountFS FAILED at umountFS\n");
		return -1;
	}

	printf("TEST closeFS + umountFS SUCCESS\n");
	return 0;
}
