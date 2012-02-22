/***************************************************************
 * File Name    : File_Encryption_Filter.cpp                   *
 * Programmer   : Jeremy O'Steen, Chris Janowiak               *
 * Purpose      : This program accepts a text to be encrypted  *
 *                from a .txt file, encrypts the file, and     *
 *                exports the encrypted file.                  *
 ***************************************************************/

/***** #Included Dependancies *****/

#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <time.h>
#include "LogicConsole.h"
#include "MersenneTwister.h"

std::string Encrypt(std::string* clearText, std::string* passPhrase, unsigned int* resultLength);

int main()
{
	/***** Add Console Object *****/

	LogicConsole *pConsole = new LogicConsole;
	pConsole -> setTitle("File Encryption Filter");  // Set title of window to "File Encryption Filter"
	pConsole -> resizeWindowV(600); // Set vertical window size to 600


	/***** Start of Program *****/

	std::cout << "This program will Encrypt a text file up to 64KiB in size.\n\n\n";
	std::string inputFileName;
	std::ifstream inputFile;
	while(!inputFile.is_open() )  // checks to see if the file is already opened
	{
		std::cout << "Enter name of file to be encrypted:\n";
		getline(std::cin, inputFileName);
		inputFile.open(inputFileName.c_str(), std::ios::in);  // Open file
		if (inputFile.fail() )  // check to make sure file could be opened successfully
		{
			std::cout << "Error opening " << inputFileName;
			std::cin.get();
			exit(1);
		}
	}
	
	const unsigned int LENGTH = 65536;  // Declare variable LENGTH as 2^16 (64KiB limit)
	std::string clearText;
	char temp[LENGTH];
	inputFile.getline(temp, LENGTH, '\0');
	clearText = std::string(temp);
	std::cout << "Please set a password for your file: \n";
	std::string passPhrase;  // Declare passPhrase variable
	passPhrase = pConsole -> cinHidden('*');  // Hide text entered with '*'


	std::cout << "\n\n\nYour text is being encrypted.\n\n\n";

	unsigned int resultLength;
	std::string cipherText;
	cipherText = Encrypt(&clearText, &passPhrase, &resultLength); // Call the function to Encrypt the text

	std::ofstream encryptedFile("encrypted_file.dat", std::ios::out | std::ios::binary); // Create the output file
	encryptedFile.write(cipherText.c_str(), resultLength);

	std::cout << "Encryption Complete\n\n";


	pConsole -> pause();  // Pauses the program
	inputFile.close();  // Closes the file
	return 0;
} // End main()


std::string Encrypt(std::string* clearText, std::string* passPhrase, unsigned int* resultLength){
	srand(time(NULL));
	
	*clearText += '\0'; // Add a '\0' to clearText so we can find the end when it's decrypted

	 // Pad out clearText with random junk characters until the length is a
	 // square of an even number. If a user passes a string that already IS
	 // the length of an even square, we pad it out to be the length of
	 // the NEXT even square, so that overwriting the last char of
	 // cipherText[] with a '\0' doesn't obliterate any part of the clearText
	 // when we decrypt. Add random characters until we get to a square of
	 // an even number for length--we always want to add at least one character

	bool exiting = false;
	double root;
	while(exiting == false)
	{
		*clearText += (char)(rand()%97 + 32);
		root = sqrt((double)(clearText->length()));
		// If root is an integer and root is even, then we're done
		if(floor(root) == root && ((int)root)%2==0) exiting=true;
	}
	int l = clearText->length();
	int leap = (int)(sqrt((double)l));

	 // "cast" the passPhrase string into an array of unsigned long to use for seeding the
	 // Mersenne Twister--better than using a single-integer seed because we access
	 // the full 2^19937-1 possibilities for number sequences
	 // First we pad the passPhrase with '\0' until it's a multiple of 4 (to make it
	 // easy on ourselves for packing four chars together into an int)

	while(passPhrase->length()%4 != 0)
		*passPhrase += '\0';
	unsigned long seed[MTRand::N];
	// Pack passPhrase[(i*4) .. (i*4)i+3] into seed[i] if passPhrase[i*4] exists
	// Else seed[i] becomes 53*i
	for(unsigned int i=0; i<MTRand::N; ++i)
	{
		if((i*4) < passPhrase->length())
			seed[i] =
				(((unsigned long)(*passPhrase)[i*4]) << 24) |
				(((unsigned long)(*passPhrase)[(i*4)+1]) << 16) |
				(((unsigned long)(*passPhrase)[(i*4)+2]) << 8) |
				((unsigned long)(*passPhrase)[(i*4)+3]);
		else
			seed[i] = 53*i;
	} // End for loop
	
	MTRand prng(seed); // Create the pseudorandom number generator by passing it the seed array

	unsigned long subBlock, currentkey; // subBlock and currentkey are 32-bit wide values to be XOR'ed to each other
	std::string cipherText = std::string(l, ' '); // Prep the cipherText[] array for writing into
	unsigned long index=0; // index is our position in the cipherText array for writing into

	// So we have a max clearText length of 2^32-1 char's which comes out to 4 GiB
	// in practice we should probably limit this to about 64KiB or 2^16 bytes

	/*********************************************************************************
	 * How the encryption algorithm works:                                           *
	 *                                                                               *
	 * Imagine the letters of the clearText arranged into a square, for example a    *
	 * 64 character clearText would have a square 8 letters by 8 letters.            *
	 * We take 2x2 subblocks of this and pack those characters into a unsigned long, *
	 * then generate a new currentkey from the Mersenne Twister. Each subBlock is    *
	 * XOR'ed with the currentkey, then written into cipherText[]. Then the last     *
	 * position in cipherText is overwritten with '\0' to make it behave as a        *
	 * proper C-style string. (We ensured that this overwriting would not destroy    *
	 * anything in the clearText by padding out the clearText so that the last       *
	 * character is ensured to be a junk character.)                                 *
	 *********************************************************************************/
	
	// The outer for-loop iterates over the rows of subblocks
	// The leap variable contains sqrt(clearText.length())==length of a side of the imaginary square
	for(int i=0; i<leap; i+=2)
	{
		// The outer for-loop iterates over the columns of subblocks
		for(int j=0; j<leap; j+=2)
		{
			// First get our currentkey from the twister
			// To be cryptographically secure, we use the most significant 8 bits
			// of 4 consecutive calls to prng.randInt() for our 32-bit integer
			currentkey =
				(prng.randInt()&0xFF000000) |
				((prng.randInt()&0xFF000000)>>8) |
				((prng.randInt()&0xFF000000)>>16) |
				((prng.randInt()&0xFF000000)>>24);
			// Next get the current 2x2 subBlock
			subBlock =
				(((unsigned long)(*clearText)[j  + i    *leap])<<24) |
				(((unsigned long)(*clearText)[j+1+ i    *leap])<<16) |
				(((unsigned long)(*clearText)[j  + (i+1)*leap])<<8)  |
				(((unsigned long)(*clearText)[j+1+ (i+1)*leap]));
			// XOR the two values together to Encrypt subBlock
			subBlock = subBlock ^ currentkey;
			// Pop chars out of subBlock into cipherText
			cipherText[index++] = (unsigned char)((subBlock&0xFF000000)>>24);
			cipherText[index++] = (unsigned char)((subBlock&0x00FF0000)>>16);
			cipherText[index++] = (unsigned char)((subBlock&0x0000FF00)>>8);
			cipherText[index++] = (unsigned char)((subBlock&0x000000FF));
			// Done, move on to the next subBlock column
		} // End inner loop
		// No processing needed in the outer loop, move on to next subBlock row
	} // End outer loop

	 // Make sure the last character of cipherText is a '\0' to ensure it's a
	 //C-style string (this is guaranteed to not overwrite anything from the clearText
	 //because we padded the clearText out to the next square of an even integer)

	*resultLength = index;
	return cipherText;
}

