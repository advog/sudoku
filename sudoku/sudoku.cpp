#include <iostream>
#include <bitset>
#include <stack>

typedef uint8_t byte;
typedef uint16_t row_flag;

struct board{
	byte filled;
	byte** vals;
	row_flag* r_flag;
	row_flag* c_flag;
	row_flag* b_flag;
};

void print_binary(row_flag number, int num_digits) {
	
	int digit;
	for (digit = num_digits - 1; digit >= 0; digit--) 
	{
		printf("%c", number & (1 << digit) ? '1' : '0');
	}
}

//fills in the row flags with available info
void static fill_row_flags(board* b) {
	
	for (byte i = 0; i < 9; i++)
	{
		for (byte j = 0; j < 9; j++)
		{
			if (b->vals[i][j] != 0)
			{
				row_flag test_bit = 0x1 << (b->vals[i][j]-1);
				
				b->r_flag[i] |= test_bit;
			
				b->c_flag[j] |= test_bit;
			
				byte b_index = (i / 3) * 3 + (j / 3);
				b->b_flag[b_index] |= test_bit;
			
				b->filled++;
			}
		}
	}
}

//adds val and updates row_flags
void static update_board(board* b, byte i, byte j, byte val) {

	//utility
	row_flag test_bit = 0x1 << (val - 1);
	byte b_index = (i / 3) * 3 + (j / 3);
	row_flag combined = b->r_flag[i] | b->c_flag[j] | b->b_flag[b_index];

	//update board and row_flags
	b->vals[i][j] = val;
	b->r_flag[i] |= test_bit;
	b->c_flag[j] |= test_bit;
	b->b_flag[b_index] |= test_bit;

	b->filled++;
}

//revert changes made by update_board to val and row_flags
void static revert_board(board* b, byte i, byte j) {
	
	//utility
	byte val = b->vals[i][j];
	row_flag remove_mask = ~(0x1 << (val - 1));
	byte b_index = (i / 3) * 3 + (j / 3);
	
	//update board and row_flags
	b->vals[i][j] = 0;
	b->r_flag[i] &= remove_mask;
	b->c_flag[j] &= remove_mask;
	b->b_flag[b_index] &= remove_mask;
	
	b->filled--;
}

//check if the board is valid and find the best place to guess next
byte static find_and_check(board* b, byte* p_i, byte* p_j) {
	
	//check if that ^ didnt invalidate previous row_flags and also find the next empty i,j with most row flags while we are at it
	byte g_i = 0;
	byte g_j = 0;
	byte g_count = 0;
	for (byte m = 0; m < 9; m++)
	{
		for (byte n = 0; n < 9; n++)
		{
			//only check if val is empty
			if (b->vals[m][n] == 0) 
			{
				//get combined flag for vals[m][n]
				byte tmp_b_index = (m / 3) * 3 + (n / 3);
				row_flag tmp_comb = b->r_flag[m] | b->c_flag[n] | b->b_flag[tmp_b_index];
				byte count = std::bitset<16>(tmp_comb).count();
				//if bit count > 8 there are no valid entries thus this is board is invalid
				if (count > 8) {  return 0; }
				//find [m][n] with least options to make search faster
				else if (count > g_count) 
				{
					g_count = count;
					g_i = m;
					g_j = n;
				}
			}

		}
	}
	//if we get here then the board is valid
	//write best i,j into p_i, p_j
	*p_i = g_i;
	*p_j = g_j;
	return 1;
}

//gets the next valid guess for a square using the row_flags and the previous guess
//if there is no valid guess it returns 10
byte static get_guess(board* b, byte i, byte j, byte prev_guess) {
	
	byte b_index = (i / 3) * 3 + (j / 3);
	row_flag combined = b->r_flag[i] | b->c_flag[j] | b->b_flag[b_index];
	
	//reverse and mask
	combined = (~combined & 0x01ff);
	
	for (byte i = 1; i < 10; i++)
	{
		if (i > prev_guess && combined & 0x0001) { return i; }
		combined = combined >> 1;
	}
	return 10;
}

void static print_board(board* b) {
	
	for (size_t i = 0; i < 9; i++)
	{
		for (size_t j = 0; j < 9; j++)
		{
			printf("%d ", b->vals[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void static print_row_flags(board* b) {
	
	for (size_t i = 0; i < 9; i++){printf("%x ", b->r_flag[i]);}
	printf("\n");
	for (size_t i = 0; i < 9; i++) { printf("%x ", b->c_flag[i]); }
	printf("\n");
	for (size_t i = 0; i < 9; i++) { printf("%x ", b->b_flag[i]); }
	printf("\n\n");
}

//please forgive me for all these callocs, I didnt know they are harmful when writing this.
void static copy_board(board* src, board* dst) {
	
	dst->vals = (byte**)calloc(9, sizeof(byte*));
	for (size_t i = 0; i < 9; i++)
	{
		dst->vals[i] = (byte*)calloc(9, sizeof(byte));
		memcpy(dst->vals[i], src->vals[i], sizeof(byte) * 9);
	}
	
	dst->r_flag = (row_flag*)calloc(9, sizeof(row_flag));
	memcpy(dst->r_flag, src->r_flag, sizeof(row_flag) * 9);

	dst->c_flag = (row_flag*)calloc(9, sizeof(row_flag));
	memcpy(dst->c_flag, src->c_flag, sizeof(row_flag) * 9);

	dst->b_flag = (row_flag*)calloc(9, sizeof(row_flag));
	memcpy(dst->b_flag, src->b_flag, sizeof(row_flag) * 9);
}

//again, pease forgive the callocs
void static initialize_board(board* b) {

	b->vals = (byte**)calloc(9, sizeof(byte*));
	for (size_t i = 0; i < 9; i++)
	{
		b->vals[i] = (byte*)calloc(9, sizeof(byte));
	}
	b->r_flag = (row_flag*)calloc(9, sizeof(row_flag));
	b->c_flag = (row_flag*)calloc(9, sizeof(row_flag));
	b->b_flag = (row_flag*)calloc(9, sizeof(row_flag));
	b->filled = 0;
}

void static delete_board(board* b) {
	
	for (size_t i = 0; i < 9; i++)
	{
		free(b->vals[i]);
	}
	free(b->vals);
	free(b->r_flag);
	free(b->c_flag);
	free(b->b_flag);
}


int main(){
	
	board b;
	board* p_b = &b;
	
	initialize_board(p_b);
	//enter starter vals
	
	//uncomment to make there be no solutions
	//p_b->vals[0][0] = 5;

	p_b->vals[0][2] = 4;

	p_b->vals[1][0] = 2;
	p_b->vals[1][2] = 5;
	p_b->vals[1][5] = 1;
	p_b->vals[1][6] = 8;

	p_b->vals[2][3] = 8;
	p_b->vals[2][8] = 3;

	p_b->vals[3][1] = 9;

	p_b->vals[4][4] = 7;
	p_b->vals[4][7] = 6;

	p_b->vals[5][0] = 1;
	p_b->vals[5][2] = 8;
	p_b->vals[5][5] = 5;
	p_b->vals[5][6] = 3;

	p_b->vals[6][1] = 3;
	p_b->vals[6][5] = 9;

	p_b->vals[7][1] = 4;
	p_b->vals[7][6] = 2;

	p_b->vals[8][0] = 9;
	p_b->vals[8][2] = 2;
	p_b->vals[8][4] = 5;
	p_b->vals[8][8] = 7;

	//fill row flags
	fill_row_flags(p_b);
	//first guess must be starter val n

	print_board(p_b);

	byte UAC;
	byte i_arr[90];
	byte j_arr[90];
	byte* guess_arr = (byte*)calloc(sizeof(byte),90);
	byte index = 0;
	
	while (p_b->filled < 81)
	{
		UAC = find_and_check(p_b, i_arr + index, j_arr + index);
		//board is valid
		if (UAC == 1) {
			guess_arr[index] = get_guess(p_b, i_arr[index], j_arr[index], 0);
		}
		//board is not valid, revert changes until we hit a space were we can guess something
		else 
		{
			do 
			{
				if (index == 0) { printf("NO SOLUTION\n"); return 0; }
				index--;
				revert_board(p_b, i_arr[index], j_arr[index]);
				guess_arr[index] = get_guess(p_b, i_arr[index], j_arr[index], guess_arr[index]);

			} while (guess_arr[index] == 10);
		}
		update_board(p_b, i_arr[index], j_arr[index], guess_arr[index]);
		index++;
	}
	print_board(p_b);
}

