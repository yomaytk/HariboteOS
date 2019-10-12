void api_end();

void main(){
	
	*((char *) 0x00102600) = 0;
	api_end();
	
}