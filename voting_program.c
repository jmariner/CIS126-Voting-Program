/*
Justin Mariner
3/30/2017
CIS126
*/

#include <stdio.h>
#include <stdlib.h>
//#include <stdarg.h>
#include <stdbool.h>
//#include <ctype.h>
#include <string.h>
#include <windows.h>
#include <time.h>

// ================================== DEFINES ==================================

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0])) // marco to get length of array

#define BLACK 0			// colors
#define DARK_BLUE 1
#define DARK_GREEN 2
#define AQUA 3
#define DARK_RED 4
#define PURPLE 5
#define GOLD 6
#define GRAY 7
#define DARK_GRAY 8
#define BLUE 9
#define GREEN 10
#define TEAL 11
#define RED 12
#define PINK 13
#define YELLOW 14
#define WHITE 15

#define BOX_CHAR '#'		// character used to create boxes
#define BOX_MAX_WIDTH 75	// max width of box
#define BOX_MAX_LINES 25	// max amount of lines a box can have
#define BOX_SPACING 3		// spacing betweeen boxes during slide animation
#define COLOR_CHAR '`'		// character used by my custom color formatting code
#define COLOR_RESET_CHAR '0'// character to reset colors
#define ERROR_LINE_TOO_LONG "!!!ERROR: LINE TOO LONG!!!" // put in place of a line that was too long to print
#define FIELD_WIDTH 25		// max width of fields / inputs
#define DEFAULT_COLOR WHITE

#define YES_NO_INVALID "Invalid yes/no entry (%s). You must specify \"yes\" or \"no\"" // error for invalid boolean input
#define TOTAL_STEPS 6		// total amount of steps in the voting process
#define CAPTCHA_MAX_ATTEMPTS 3 // max amount of tries at the captcha

// ================================== GLOBAL VARIABLES ==================================

HANDLE console;			// these are a bunch of variables for the color and box formatting.
int colorMapping[128];	// <-- this is worth explaining: it's an array with the indicies being character codes to act
char* horizonalLine;	//		sort of like a map for color code letters to color numbers
char colorReset[] = {COLOR_CHAR, COLOR_RESET_CHAR, '\0'};
char* currentTextLines[BOX_MAX_LINES];
int currentLineCount = -1;
int boxWidth;

char* firstName;		// first name
char* lastName;			// last name
int userAge;			// age
char* userState;		// state code
bool isCitizen;			// whether or not user is a citizen
bool hasFelony;			// whether or not user has a felony
bool hasRightsRestored;	// whether or not user has had their rights restored after a felony
bool captchaCorrect;	// whether or not the user got the captcha correct
int candidateSelection;	// chosen candidate number
int currentProgress;	// current step out of STEPS_COUNT
bool allowedToVote;	// whether or not user is allowed to vote
bool returningToStep;	// set true when user is returning to a step
bool programDone; // set true when main program loop is complete
char* notAllowedReasons[TOTAL_STEPS]; 	// array of all reasons user was not allowed to vote, if any
int notAllowedReasonCount;				// amount of reasons not allowed to vote

// these are all used to track whether or not each option had been set yet
bool firstNameSet, lastNameSet, ageSet, stateSet, citizenshipSet, felonySet, rightsRestoredSet;

char* tempInput; // input variable used by all input reads. this is used to it can be validated before setting the actual value

// available captcha options and a parallel array of their answers
char* CAPTCHA_OPTIONS[] = { "2 + 2", "1 + 2", "2 + 3", "5 + 2", "7 + 1" };
int CAPTCHA_ANSWERS[] = {4, 3, 5, 7, 8};
int CAPTCHA_COUNT = ARRAY_LEN(CAPTCHA_ANSWERS);

char* VALID_STATES[] = {	// valid state codes. normal 50 + PR and DC
	"AL", "AK", "AZ", "AR", "CA", "CO", "CT", "DE", "DC", "FL", "GA", "HI", "ID", "IL", "IN", "IA", "KS", "KY", "LA", "ME", "MD", "MA", "MI", "MN", "MS", "MO", 
	"MT", "NE", "NV", "NH", "NJ", "NM", "NY", "NC", "ND", "OH", "OK", "OR", "PA", "PR", "RI", "SC", "SD", "TN", "TX", "UT", "VT", "VA", "WA", "WV", "WI", "WY"
};
int STATE_COUNT = ARRAY_LEN(VALID_STATES);

char* CANDIDATE_OPTIONS[] = {	// candidate choices
	"Joe Biden",
	"Bob McBobby",
	"Pedro Ringer",
	"Mark Dreyfus"
};
int CANDIDATE_COUNT = ARRAY_LEN(CANDIDATE_OPTIONS);

// booleans can be read as 0 for false or 1 for true. this uses those as array indices
char* BOOLEAN_STRINGS[] = {"no", "yes"};

// =========================== FUNCTION PROTOTYPES =======================

// main
void prepareInfoBox();
char* makeProgressBar();
void printPrompt(char* promptMessage);
void showError(char* errorMessage, ...);

// voting steps
void runStep(int stepIndex);
void inputFirstName();
void inputLastName();
void inputAge();
void inputState();
void inputCitizenship();
void inputFelonyStatus();
void inputCaptcha();
void inputCandidateChoice();

// utilities
void clearScreen();
void silentPause();
char* booleanToString(bool booleanValue);
void stringToUpper(char inputString[]);
bool isInteger(char* inputString);

// display stuff
void startBox();
void printLines();
void addBlankLine();
void addHorizontalLine();
void addCenterLine(char* formatLine, ...);
void setCenterLine(int lineIndex, char* formatLine, ...);
void addLeftLine(char* formatLine, ...);
void setLeftLine(int lineIndex, char* formatLine, ...);
char* makeBoxLine(bool isCentered, char* format, va_list argList);
char* formatString(char* format, va_list argList);
void colorPrint(char* format, ...);
char* stripColors(char* formattedString);
void setColor(int colorId);
int makeColor(int foregroundId, int backgroundId);
void resetColor();
void printFlag(char* addedMessage);

void initDisplay() {	// this function initializes everything about the display
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	
	console = GetStdHandle(STD_OUTPUT_HANDLE);
	
	// store the console width so the box width can be sized accordingly
	GetConsoleScreenBufferInfo(console, &consoleInfo);
	boxWidth = consoleInfo.dwSize.X-1;
	
	if (boxWidth > BOX_MAX_WIDTH)
		boxWidth = BOX_MAX_WIDTH;
	
	// initialize the color map values to -1 and map some colors
	memset(colorMapping, -1, 128 * sizeof(int));
	colorMapping['r'] = RED;
	colorMapping['g'] = GREEN;
	colorMapping['b'] = BLUE;
	colorMapping['t'] = TEAL;
	colorMapping['a'] = AQUA;
	colorMapping['d'] = GOLD;
	colorMapping['p'] = makeColor(0, GREEN); // progress color
	colorMapping['q'] = makeColor(0, GRAY); // progress missing colors
	colorMapping[COLOR_RESET_CHAR] = DEFAULT_COLOR;
	
	// create a horizontal line of box characters
	horizonalLine = (char*) malloc(boxWidth+2);
	memset(horizonalLine, BOX_CHAR, boxWidth);
	horizonalLine[boxWidth] = '\n';
	horizonalLine[boxWidth+1] = '\0';
	
	// initially reset the color to the set default
	resetColor();
}

int main() {
	bool wantsToReturn; // if the user wants to return to a step
	bool wantsToRestart; // if the user wants to restart the program
	int stepToReturn; // step the user is returning to
	int problemCounter; // loop control variable for printing reasons user cannot vote
	int booleanResult; // -1, 0, or 1 result from parseBoolean(...)
	
	// allocate space for global strings
	firstName = (char*) malloc(FIELD_WIDTH+1);
	lastName = (char*) malloc(FIELD_WIDTH+1);
	userState = (char*) malloc(FIELD_WIDTH+1);
	tempInput = (char*) malloc(255);
	
	// set the random seed to the current time (random enough)
	srand(time(NULL));
	
	initDisplay();
	
	do { // main code loop, controlled by wantsToRestart
	
		// start all these flags at false
		programDone = wantsToReturn = wantsToRestart = firstNameSet = lastNameSet = ageSet = stateSet = citizenshipSet = felonySet = rightsRestoredSet = false;
	
		// print initial information
		startBox();
		addCenterLine("Welcome to my voting program!");
		addBlankLine();
		addLeftLine("Here, you can figure out if you are eligible to");
		addLeftLine("vote in the United States and can choose your");
		addLeftLine("preferred candidate for the election.");
		addBlankLine();
		addHorizontalLine();
		addCenterLine("Instructions and Tips");
		addBlankLine();
		addLeftLine(" - Answer questions according to their instructions");
		addLeftLine(" - Answer honestly");
		addLeftLine(" - For \"yes or no\"-type questions, enter \"`tyes`0\"/\"`ty`0\" or \"`tno`0\"/\"`tn`0\"");
		addBlankLine();
		addCenterLine("Press any key to begin...");
		printLines();
		
		silentPause();
		clearScreen();
	
		// loop through steps in order
		for (currentProgress = 0; currentProgress < TOTAL_STEPS; currentProgress++)
			runStep(currentProgress);
		
		do { // return to steps loop, controlled by wantsToReturn
		
			while (1) {
				// we're done the steps, ask if it's correct
				prepareInfoBox();
				printPrompt("Is this information correct?");
				scanf("%s", tempInput);
				
				booleanResult = parseBoolean(tempInput);
				if (booleanResult == -1)
					showError(YES_NO_INVALID, tempInput);
				else
					break;
			}
			
			wantsToReturn = (bool) !booleanResult;
			
			if (wantsToReturn) {
				
				returningToStep = true;
				
				while (1) {
					
					prepareInfoBox();
					printPrompt("Enter the option you would like to edit");
					scanf("%s", tempInput);
					
					if (!isInteger(tempInput)) {
						showError("Your must enter the number of the option to edit.");
					}
					else {
						stepToReturn = strtol(tempInput, NULL, 10);
						if (stepToReturn < 1 || stepToReturn > TOTAL_STEPS)
							showError("Invalid option (%d) entered", stepToReturn);
						else
							break;
					}
				}
				
				returningToStep = false;
				currentProgress = stepToReturn-1;
				runStep(currentProgress);
				currentProgress = TOTAL_STEPS;
			
			}
			
		} while (wantsToReturn);
		
		notAllowedReasonCount = 0;
		if (userAge < 18)
			notAllowedReasons[notAllowedReasonCount++] = "You are under the age of 18";
		if (!isCitizen)
			notAllowedReasons[notAllowedReasonCount++] = "You are not a citizen of the United States";
		if (hasFelony && !hasRightsRestored)
			notAllowedReasons[notAllowedReasonCount++] = "You are a convicted felon who has not had their rights restored";
		
		allowedToVote = notAllowedReasonCount == 0;
		
		while (1) {
		
			if (allowedToVote) {
				
				if (!programDone)
					inputCaptcha();
				
				if (captchaCorrect) {
					
					if (!programDone) {
						startBox();
						addCenterLine("Captcha Correct");
						addBlankLine();
						addCenterLine("Correct. You may now vote on your candidate of choice.");
						addCenterLine("Press any key to continue and enter your vote...");
						addBlankLine();
						printLines();
						
						silentPause();
						
						inputCandidateChoice();
					}
					
					startBox();
					addCenterLine("Vote Submitted");
					addBlankLine();
					addCenterLine("Your vote for `t%s`0 has been recorded.", CANDIDATE_OPTIONS[candidateSelection]);
				}
				else {
					startBox();
					addCenterLine("Captcha Failed");
					addBlankLine();
					addCenterLine("You failed to complete the captcha and are now unable to vote.");
				}
				
			}
			else {
				startBox();
				addCenterLine("Ineligible to Vote");
				addBlankLine();
				addLeftLine("You are not eligible to vote for the following reasons:");
				for (problemCounter = 0; problemCounter < notAllowedReasonCount; problemCounter++)
					addLeftLine(" - %s", notAllowedReasons[problemCounter]);
			}
			
			programDone = true;
			
			addBlankLine();
			addHorizontalLine();
			
			printPrompt(allowedToVote && captchaCorrect ?
				"Would you like to restart the program and vote again?" :
				"Would you like to restart the program?"
			);
			
			scanf("%s", tempInput);
			
			booleanResult = parseBoolean(tempInput);
			
			if (booleanResult == -1)
				showError(YES_NO_INVALID, tempInput);
			else
				break;
		}
		
		wantsToRestart = (bool) booleanResult;
		
	} while (wantsToRestart);
	
	clearScreen();
	
	printFlag(allowedToVote && captchaCorrect ? "Thank you for voting" : "Thank you for your time");
	
	printf("Press any key to close the program...");
	silentPause();
	return 0;
}

void prepareInfoBox() {
	int curStepNum = currentProgress + 1;
	char* ageString = (char*) malloc(8);
	char* highlightColor = "`d";
	
	startBox();
	addCenterLine("User Information");
	
	addLeftLine("%s%sFirst Name: `t%s",
		curStepNum == 1 ? highlightColor : "",
		returningToStep ? "(1) " : "",
		firstNameSet ? firstName : ""
	);
	
	addLeftLine("%s%sLast Name: `t%s",
		curStepNum == 2 ? highlightColor : "",
		returningToStep ? "(2) " : "",
		lastNameSet ? lastName : ""
	);
	
	if (ageSet) sprintf(ageString, "%d", userAge);
	else ageString = "";
	
	addLeftLine("%s%sAge: `t%s",
		curStepNum == 3 ? highlightColor : "",
		returningToStep ? "(3) " : "",
		ageString
	);
	
	addLeftLine("%s%sState: `t%s",
		curStepNum == 4 ? highlightColor : "",
		returningToStep ? "(4) " : "",
		stateSet ? userState : ""
	);
		
	addLeftLine("%s%sIs citizen? `t%s",
		curStepNum == 5 ? highlightColor : "",
		returningToStep ? "(5) " : "",
		citizenshipSet ? booleanToString(isCitizen) : ""
	);
	
	addLeftLine("%s%sHas a felony? `t%s",
		!felonySet && curStepNum == 6 ? highlightColor : "",
		returningToStep ? "(6) " : "",
		felonySet ? booleanToString(hasFelony) : ""
	);
	
	if (felonySet && hasFelony) {
		addLeftLine("     %sRights Restored? `t%s",
			curStepNum == 6 ? highlightColor : "",
			rightsRestoredSet ? booleanToString(hasRightsRestored) : ""
		);
	}
	
	addBlankLine();
	addLeftLine("Progress: %s", makeProgressBar());
	addHorizontalLine();
}

char* makeProgressBar() {
	int barWidth = boxWidth - 16;
	int progressSize = ((float) currentProgress / TOTAL_STEPS) * barWidth;
	char* progressString = (char*) malloc(128);
	
	sprintf(progressString, "`p%*s`q%*s`0",
		progressSize, "",
		barWidth - progressSize, "");
	
	return progressString;
}

void printPrompt(char* promptMessage) {
	int leftPadding = boxWidth/2 - 10;
	
	addBlankLine();
	addCenterLine(promptMessage);
	addBlankLine();
	printLines();
	
	printf("\n%*.s> ", leftPadding, "");
}

void showError(char* errorMessage, ...) {
	char* formattedMessage;
	char* coloredMessage = (char*) malloc(255);
	va_list argList;
	
	va_start(argList, errorMessage);
	formattedMessage = formatString(errorMessage, argList);
	va_end(argList);
	
	sprintf(coloredMessage, "`r%s", formattedMessage);
	
	startBox();
	addCenterLine("An error has occurred");
	addBlankLine();
	addCenterLine(coloredMessage);
	addBlankLine();
	addCenterLine("Press any key to try again...");
	printLines();
	
	silentPause();
	
}

void runStep(int stepIndex) {
	switch (stepIndex) {
		case 0:
			inputFirstName();
			break;
		case 1:
			inputLastName();
			break;
		case 2:
			inputAge();
			break;
		case 3:
			inputState();
			break;
		case 4:
			inputCitizenship();
			break;
		case 5:
			inputFelonyStatus();
			break;
		default:
			break; // should never happen since case numbers are validated before coming to this function
	}
}

void inputFirstName() {
	
	while (1) {
		prepareInfoBox();
		printPrompt("Enter your first name");
		scanf("%s", firstName);
		
		if (strlen(firstName) > FIELD_WIDTH)
			showError("Sorry, your name is too long!");
		else
			break;
	}

	firstNameSet = true;
}

void inputLastName() {
	
	while (1) {
		prepareInfoBox();
		printPrompt("Enter your last name");
		scanf("%s", lastName);
		
		if (strlen(lastName) > FIELD_WIDTH)
			showError("Sorry, your name is too long!");
		else
			break;
	}
	
	lastNameSet = true;
}

void inputAge() {
	int tempAge;
	
	while (1) {
		prepareInfoBox();
		printPrompt("Enter your age");
		scanf("%s", tempInput);
		
		if (!isInteger(tempInput)) {
			showError("You must input a number for age.");
		}
		else {
			tempAge = strtol(tempInput, NULL, 10);
			if (tempAge < 0 || tempAge > 999)
				showError("Sorry, that age is invalid.");
			else
				break;
		}
	}
	
	userAge = tempAge;
	ageSet = true;
}

void inputState() {
	int stateCounter;
	bool stateFound = false;
	
	while (1) {
		prepareInfoBox();
		printPrompt("Enter your 2-letter state code");
		scanf("%s", userState);
		stringToUpper(userState);
		
		for (stateCounter = 0; stateCounter < STATE_COUNT; stateCounter++) {
			if (strcmp(VALID_STATES[stateCounter], userState) == 0) {
				stateFound = true;
				break;
			}
		}
		
		if (!stateFound)
			showError("The state you entered (%s) is invalid.", userState);
		else
			break;
	}
	
	stateSet = true;
}

void inputCitizenship() {
	int booleanResult;
	
	while (1) {
		prepareInfoBox();
		printPrompt("Are you currently a citizen of the United States?");
		scanf("%s", tempInput);
		
		booleanResult = parseBoolean(tempInput);
		if (booleanResult == -1)
			showError(YES_NO_INVALID, tempInput);
		else
			break;
	}
	
	isCitizen = (bool) booleanResult;
	citizenshipSet = true;
}

void inputFelonyStatus() {
	int booleanResult;
	
	// reset these flag for this one since it's a two-part step
	felonySet = false;
	hasRightsRestored = false;
	
	while (1) {
		prepareInfoBox();
		printPrompt("Have you ever been convicted of a felony?");
		scanf("%s", tempInput);

		booleanResult = parseBoolean(tempInput);
		if (booleanResult == -1)
			showError(YES_NO_INVALID, tempInput);
		else
			break;
	}
	
	felonySet = true;
	hasFelony = (bool) booleanResult;
	
	if (hasFelony) {
		
		while (1) {
			prepareInfoBox();
			printPrompt("Have you had your rights restored?");
			scanf("%s", tempInput);
	
			booleanResult = parseBoolean(tempInput);
			if (booleanResult == -1)
				showError(YES_NO_INVALID, tempInput);
			else
				break;
		}
		
		hasRightsRestored = (bool) booleanResult;
	}
	else {
		hasRightsRestored = false;
	}
	
	rightsRestoredSet = true;
}

void inputCaptcha() {
	int leftPadding = boxWidth/2 - 10;
	int timesFailed = 0;
	int captchaOption = rand() % CAPTCHA_COUNT;
	int captchaInput;
	int triesRemaining;
	
	while (1) {
		
		startBox();
		addBlankLine();
		addCenterLine("Congratulations! You are eligible to vote in this election!");
		addCenterLine("Before voting, please answer the following captcha");
		addCenterLine("to prove you are not a robot.");
		addBlankLine();
		printLines();
		
		printf("\n%*s %s = ", leftPadding, "", CAPTCHA_OPTIONS[captchaOption]);
		scanf("%s", tempInput);
		
		if (!isInteger(tempInput)) {
			showError("You must enter a number. This has not counted against you.");
		}
		else {
			
			captchaInput = strtol(tempInput, NULL, 10);
			
			if (captchaInput != CAPTCHA_ANSWERS[captchaOption]) {
				timesFailed++;
				
				if (timesFailed >= CAPTCHA_MAX_ATTEMPTS) {
					showError("Sorry, you have ran out of attempts.");
					captchaCorrect = false;
					break;
				}
				else {
					triesRemaining = CAPTCHA_MAX_ATTEMPTS - timesFailed;
					showError("You have entered the incorrect answer. You have %d %s remaining.", triesRemaining, triesRemaining == 1 ? "try" : "tries");
					captchaOption = rand() % CAPTCHA_COUNT;
				}
			}
			else {
				captchaCorrect = true;
				break;
			}
			
		}
	}
}

void inputCandidateChoice() {
	int candCounter;
	int tempChoice;
	
	while (1) {

		startBox();
		addCenterLine("Your candidate options are as follows:");
		addBlankLine();
		
		for (candCounter = 0; candCounter < CANDIDATE_COUNT; candCounter++)
			addLeftLine("(%d) %s", candCounter+1, CANDIDATE_OPTIONS[candCounter]);
		
		addBlankLine();
		addHorizontalLine();
		
		printPrompt("Enter your choice");
		scanf("%s", tempInput);
		
		if (!isInteger(tempInput)) {
			showError("You must input the number for your candidate.");
		}
		else {
			tempChoice = strtol(tempInput, NULL, 10);
			if (tempChoice <= 0 || tempChoice > CANDIDATE_COUNT)
				showError("Sorry, that candidate does not exist.");
			else
				break;
		}
	
	}
	
	candidateSelection = tempChoice - 1;
	
}

// ==================== UTILITY FUNCTIONS ==================================

void clearScreen() {
	system("cls");
}

void silentPause() {
	system("pause > NUL");
}

char* booleanToString(bool booleanValue) {
	return BOOLEAN_STRINGS[(int) booleanValue];
}

int parseBoolean(char* booleanString) {
	
	stringToUpper(booleanString);
	if (strcmp(booleanString, "Y") == 0 || strcmp(booleanString, "YES") == 0)
		return 1;
	else if (strcmp(booleanString, "N") == 0 || strcmp(booleanString, "NO") == 0)
		return 0;
	else
		return -1;
}

void stringToUpper(char inputString[]) {
	int charCounter;
	int stringLen = strlen(inputString);
	
	for (charCounter = 0; charCounter < stringLen; charCounter++) {
		inputString[charCounter] = toupper(inputString[charCounter]);
	}
}

bool isInteger(char* inputString) {
	int strLength = strlen(inputString);
	int charCounter;
	char currentChar;
	
	for (charCounter = 0; charCounter < strLength; charCounter++) {
		
		currentChar = inputString[charCounter];
		
		if (!isdigit(currentChar) && currentChar != '-')
			return false;
		if (currentChar == '-' && charCounter > 0)
			return false;
	}
	return true;
}

// ======================= DISPLAY FUNCTIONS ============================

void startBox() {
	currentLineCount = 0;
	addHorizontalLine();
}

void printLines() {
	int lineCounter;
	
	addHorizontalLine();
	
	clearScreen();
	
	for (lineCounter = 0; lineCounter < currentLineCount; lineCounter++)
		colorPrint(currentTextLines[lineCounter]);
}

void addBlankLine() {
	addCenterLine("");
}

void addHorizontalLine() {
	currentTextLines[currentLineCount++] = horizonalLine;
}

void addCenterLine(char* formatLine, ...) {
	va_list argList;
	va_start(argList, formatLine);
	currentTextLines[currentLineCount++] = makeBoxLine(true, formatLine, argList);
	va_end(argList);
}

void addLeftLine(char* formatLine, ...) {
	va_list argList;
	va_start(argList, formatLine);
	currentTextLines[currentLineCount++] = makeBoxLine(false, formatLine, argList);
	va_end(argList);	
}

char* makeBoxLine(bool isCentered, char* format, va_list argList) {
	char* resultString = (char*) malloc(255);
	char* formattedString;
	int stringLen, linePadding;
	
	formattedString = formatString(format, argList);
	stringLen = strlen(stripColors(formattedString));
	
	if (stringLen > boxWidth-4)
		return makeBoxLine(isCentered, ERROR_LINE_TOO_LONG, NULL);
	
	if (isCentered) {
		
		linePadding = ((boxWidth - stringLen) / 2) - 1;
		sprintf(resultString, "%c%*s%s%*s%s%c\n",
			BOX_CHAR,
			linePadding, " ",
			formattedString,
			stringLen % 2 != boxWidth % 2 ? linePadding + 1 : linePadding, " ",
			colorReset, // append color reset code
			BOX_CHAR);		
	}
	else {
		
		linePadding = boxWidth - stringLen - 3;
		sprintf(resultString, "%c %s%*s%s%c\n",
			BOX_CHAR,
			formattedString,
			linePadding, " ",
			colorReset, // append color reset code
			BOX_CHAR);
	}
	
	return resultString;
}

char* formatString(char* format, va_list argList) {
	char* formattedString = (char*) malloc(255);
	vsprintf(formattedString, format, argList);
	return formattedString;
}

void colorPrint(char* format, ...) {
	char DELIM[] = {COLOR_CHAR, '\0'};
	char* formattedString;
	char* currentToken;
	char* savePointer;
	bool coloredFirst;
	char firstChar;
	int curColor;
	va_list argList;
	
	va_start(argList, format);
	formattedString = formatString(format, argList);
	va_end(argList);
	
	coloredFirst = strlen(formattedString) > 0 && formattedString[0] == COLOR_CHAR;
	
	currentToken = strtok_r(formattedString, DELIM, &savePointer);
	
	while (currentToken != NULL) {
		
		if (!coloredFirst) { // this is true only for the first token in a line not prefixed by a format code
			printf("%s", currentToken);
			coloredFirst = true;
		}
		else {
			firstChar = currentToken[0]; // first character, which was after the color format symbol, is the color code letter
			curColor = colorMapping[(int) firstChar];
			
			if (curColor > -1)
				setColor(curColor);
				
			if (strlen(currentToken) > 1)		// print from the 2nd letter onward.
				printf("%s", &currentToken[1]); // since a string is represented by a char pointer, this gets the
												// second char and gets its address to print from there
		}
		
		currentToken = strtok_r(NULL, DELIM, &savePointer);
	}
	
	resetColor();
}

char* stripColors(char* formattedString) {
	int origLen = strlen(formattedString);
	char* resultString = (char*) malloc(origLen); // stripped will be smaller, so original length is enough for sure
	int curIndex;
	int resultIndex = 0;
	char currentChar, nextChar;
	
	for (curIndex = 0; curIndex < origLen; curIndex++) {
		
		currentChar = formattedString[curIndex];
		
		if (currentChar == COLOR_CHAR && curIndex < origLen-1) {
			nextChar = formattedString[curIndex+1];
			if (colorMapping[(int) nextChar] > -1) {
				curIndex++;
				continue;
			}
		}
		
		resultString[resultIndex++] = currentChar;
		
	}
	
	resultString[resultIndex] = '\0';
	
	return resultString;
	
}

void setColor(int colorId) {
	if (colorId > -1)
		SetConsoleTextAttribute(console, colorId);
}

int makeColor(int foregroundId, int backgroundId) {
	if (foregroundId > 15 || backgroundId > 15)
		return DEFAULT_COLOR;
	
	return foregroundId + 16*backgroundId;
}

void resetColor() {
	setColor(DEFAULT_COLOR);
}

void printFlag(char* addedMessage) {
	const int FLAG_LINES = 13;
	const int FLAG_WIDTH = 60;
	const int BLUE_WIDTH = FLAG_WIDTH * 0.4;
	const int BLUE_HEIGHT = 7;
	const int POLE_SIZE = 8;
	const int POLE_COLOR = makeColor(0, DARK_RED);
	bool printMessage = addedMessage != NULL;
	int messageLen = printMessage ? strlen(addedMessage) : 0;
	int horizCounter, verticalCounter;
	int currentColor;
	
	if (messageLen > FLAG_WIDTH) {
		printFlag(ERROR_LINE_TOO_LONG);
		return;
	}
	
	putchar(' ');
	setColor(POLE_COLOR);
	puts(" ");
	puts("   ");
	
	for (verticalCounter = 0; verticalCounter < FLAG_LINES; verticalCounter++) {
	
		resetColor();
		putchar(' ');
		setColor(POLE_COLOR);
		putchar(' ');
		
		for (horizCounter = 0; horizCounter < FLAG_WIDTH; horizCounter++) {
			
			if (horizCounter < BLUE_WIDTH && verticalCounter < BLUE_HEIGHT) {
				if (verticalCounter == 0 || verticalCounter == BLUE_HEIGHT-1 || // first and last, vertically
						horizCounter == 0 || horizCounter == BLUE_WIDTH-1 ||	// first and last, horizontally
						horizCounter % 2 != verticalCounter % 2) {		// vertical and horizontal have opposite parity
					currentColor = BLUE;
				}
				else {
					currentColor = WHITE;
				}
			}
			else {
				if (verticalCounter % 2 == 0)
					currentColor = RED;
				else
					currentColor = WHITE;
			}
			
			setColor(makeColor(0, currentColor));
			putchar(' ');
		}
		
		printf("\n");
		
	}
	
	for (verticalCounter = 0; verticalCounter < POLE_SIZE; verticalCounter++) {
		resetColor();
		putchar(' ');
		setColor(POLE_COLOR);
		putchar(' ');
		
		if (printMessage && verticalCounter == POLE_SIZE/2) {
			resetColor();
			printf("%*s%s\n", (FLAG_WIDTH - messageLen)/2, " ", addedMessage);
		}
		else {
			printf("\n");
		}
	}
	
	resetColor();
}

