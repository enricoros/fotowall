// Call this function first (just once is needed), before calling sonix_decompress
void sonix_decompress_init(void);

// decompresses data at inp until a full image of widthxheight has been written to outp
int sonix_decompress(int width, int height, unsigned char *inp, unsigned char *outp);

// counter to detect presence of currently unknown huffman codes
extern int sonix_unknown;
