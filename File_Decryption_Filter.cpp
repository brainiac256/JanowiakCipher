/***************************************************************
 * File Name    : File_Decryption_Filter.cpp                   *
 * Programmer   : Jeremy O'Steen, Chris Janowiak               *
 * Purpose      : This program accepts a text to be decrypted  *
 *                from a .txt file, decrypts the file, and     *
 *                exports the decrypted file.                  *
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

std::string decrypt(std::string* ciphertext, std::string* passphrase, unsigned int messagelength);

int main()
{
	/***** Add Console Object *****/

	LogicConsole *pConsole = new LogicConsole;
	pConsole -> setTitle("File Decryption Filter");  // Set title of window to "File Encryption Filter"
	pConsole -> resizeWindowV(600); // Set vertical window size to 600


	/***** Start of Program *****/

	std::cout << "This program will decrypt a text file up to 64KiB in size.\n\n\n";
	std::string inputFileName;
	std::ifstream inputFile;
	while(!inputFile.is_open() )  // Checks to see if the file is already opened
	{
		std::cout << "Enter name of file to be decrypted:\n";
		getline(std::cin, inputFileName);
		inputFile = std::ifstream(inputFileName.c_str(), std::ios::in | std::ios::binary);  // Open file
		if (inputFile.fail() )  // Check to make sure file could be opened successfully
		{
			std::cout << "Error opening " << inputFileName;
			std::cin.get();
			exit(1);
		}
	}
	
	const unsigned int LENGTH = 65536;  // Declare variable LENGTH as 2^24 (16MiB limit)
	std::string cipherText((std::istreambuf_iterator<char>(inputFile)), 
    std::istreambuf_iterator<char>());
	
	std::cout << "Please enter the password for your file: \n";
	std::string passPhrase;  // Declare passPhrase variable
	passPhrase = pConsole -> cinHidden('*');  // Hide text entered with '*'


	std::cout << "\n\n\nYour text is being decrypted.\n\n\n";

	std::string clearText = decrypt(&cipherText, &passPhrase, cipherText.length()); // Call the function to decrypt the text

	std::ofstream decryptedFile("decrypted_file.txt", std::ios::out); // Create & open the output file
	decryptedFile << clearText.c_str();

	std::cout << "Decryption Complete\n\n";
	std::cout << clearText.c_str() << std::endl;

	pConsole -> pause();  // Pauses the program
	inputFile.close();  // Closes the file
	return 0;
} // End main()

std::string decrypt(std::string* ciphertext, std::string* passphrase, unsigned int messagelength){
	// First we seed our PRNG properly, this code is identical to the seeding snippet in encrypt()
	while(passphrase->length()%4 != 0)
		*passphrase += '\0';
	unsigned long seed[MTRand::N];
	// Pack passphrase[(i*4) .. (i*4)i+3] into seed[i] if passphrase[i*4] exists
	
	for(unsigned int i = 0; i < MTRand::N; ++i){
		if((i*4) < passphrase->length())
			seed[i] =
				(((unsigned long)(*passphrase)[i*4]) << 24) +
				(((unsigned long)(*passphrase)[(i*4)+1]) << 16) +
				(((unsigned long)(*passphrase)[(i*4)+2]) << 8 ) +
				((unsigned long)(*passphrase)[(i*4)+3]);
		else // Else seed[i] becomes 53*i 
			seed[i] = 53*i;
	} // End for loop

	// Create the pseudorandom number generator by passing it the seed array
	MTRand prng(seed);
	// All this will result in getting a pseudorandom number generator with THE EXACT SAME STATE
	// as the one we used to encrypt the file, which is important.

	/********************************************************************************************
	 * Decryption algorithm:                                                                    *
	 *                                                                                          *
	 * Every 4 characters is an encrypted subblock of the original ciphertext, so given that we *
	 * iterate through all the characters in steps of 4, we'll be able to place each character  *
	 * in cleartext[] knowing our current index, the square root of the length (that was our    *
	 * side length for the imaginary square when encrypting), and our current column number in  *
	 * the grid of subblocks in the imaginary square.                                           *
	 ********************************************************************************************/

	/********************************************************************************************
	 * Demonstration:                                                                           *
	 *                                                                                          *
	 * Message is "ABCDEFGHIJKLMNOP"                                                            *
	 * Imaginary square:                                                                        *
	 *                                                                                          *
	 *  column: 0 |1                                                                            *
	 *          AB|CD                                                                           *
	 *          EF|GH                                                                           *
	 *          --|--                                                                           *
	 *          IJ|KL                                                                           *
	 *          MN|OP                                                                           *
	 * Order of output characters is "ABEFCDGHIJMNKLOP"                                         *
	 * Let index be the position of our 'cursor' in the output message                          *
	 * Let column be our subblock column number                                                 *
	 * Let sidelength be our side length                                                        *
	 * For each set of 4 characters beginning with the character at index                       *
	 * The first character goes in (index - 2*column)                                           *
	 * The next character goes in  (index+1 - 2*column)                                         *
	 * The third character goes in (index + sidelength - 2*column)                              *
	 * The last character goes in  (index+1 + sidelength - 2*column)                            *
	 * That is to say, we should set cleartext[index-2*column]     = ciphertext[index],         *
	 *                               cleartext[index+1 - 2*column] = ciphertext[index+1]        *
	 * and so on. (After we XOR them with the proper currentkey, of course!)                    *
	 * This is true for a message length of any square of an even integer (so that our          *
	 * side lengths remain integers)--which we have ensured is the case via the cleartext       *
	 * padding in the encrypt() function.                                                       *
	 ********************************************************************************************/

	// We shouldn't have to prep the ciphertext[] string at all because it will be the output of
	// the encrypt() function. Also, we've made sure the ciphertext will end in a '\0' so that's
	// a convenient way of measuring when to stop trying to decode.
	// this call to floor SHOULDN'T be necessary but you should NEVER trust user-input data if you
	// have any sort of a choice at all.

	int sidelength = (int)(floor(sqrt((double)messagelength)));
	unsigned int index = 0;
	int column = 0;
	unsigned long currentkey=0;
	unsigned long subblock = 0;

	// New string, make it full of spaces and the proper length
	std::string cleartext = std::string(messagelength + 4, ' ');

	// We're certain that our ciphertext contains AT LEAST 4 char's: this check will have to be
	// done in the calling function, so we can run the loop at least once.
	bool exiting = false;
	for(index = 0; index < messagelength; index += 4)
	{
		// Get the next currentkey from the prng with prng.nextInt()
		// This is copied straight from the corresponding snippet in encrypt()
		currentkey = ( prng.randInt()&0xFF000000) |
					 ((prng.randInt()&0xFF000000) >> 8) |
					 ((prng.randInt()&0xFF000000) >> 16) |
					 ((prng.randInt()&0xFF000000) >> 24);

		// Build our current subblock, much easier than before since we just take 4 consecutive chars from 
		subblock = ((unsigned long)((ciphertext->at(index) & 0x000000FF)) << 24)|
				   ((unsigned long)((ciphertext->at(index+1)&0x000000FF)) << 16)|
				   ((unsigned long)((ciphertext->at(index+2)&0x000000FF)) << 8 )|
				   ((unsigned long)((ciphertext->at(index+3)&0x000000FF)) << 0 );

		// Decrypt with XOR
		subblock = subblock ^ currentkey;

		// Characters go one by one into their proper places in cleartext[]
		cleartext[index - 2 * column] = (unsigned char)((subblock&0xFF000000)>>24);
		cleartext[index + 1 - 2 * column] = (unsigned char)((subblock&0x00FF0000)>>16);
		cleartext[index + sidelength - 2 * column] = (unsigned char)((subblock&0x0000FF00)>>8);
		cleartext[index + sidelength + 1 - 2 * column] = (unsigned char)((subblock&0x000000FF));

		// Increment column and wrap around if it exceeds sidelength/2 since each column is 2 char's wide
		++column;
		column = column % (sidelength / 2);
	}
	return cleartext;
}