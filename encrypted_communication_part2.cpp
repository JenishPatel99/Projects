// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Name = Mattheas Jamieson, Jenish Patel
// Student ID: 1601689 , 1572027
// CMPUT 274, FALL 2019
// Major Assignment: Encryption Communication Part 2
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include <Arduino.h>
#include <math.h>

// Constants and state defintion.
const int analogpin = 1;
const int serverPin = 13;
enum ConnectionState {
  	START, WAITINGFORACK, DATAEXCHANGE, LISTEN, WAITINGFORKEY, WAITINGFORKEY2
};

// Setups up serial communication and INPUT pins.
void setup() {
	init();
	Serial.begin(9600);
	Serial3.begin(9600);
	pinMode(analogpin, INPUT);
	pinMode(serverPin, INPUT);
}

/*
	Description:
	Checks if number n is a prime number. Code adopted from
	morning problem PRIMALITY.

	Return: true is prime, else false
*/
bool check_prime(uint32_t n) {
    bool check = true;

    for (uint32_t i = 2; i <= sqrt(n); i++) {
        if (n % i == 0 && check != false) {
            check = false;
        }
    }

    if (check == true && n != 1) {
        return true;
    } else {
        return false;
    }

    return 0;
}

/*
	Description:
	Generates a random number between a specified domain. Using voltage readings
	from analog pin 1 the function will generate a n bit integer.

	Return uint32_t randonNumber
*/
uint32_t generate_random_number(int n) {
	int voltageReading;
	int recordLSB = 0;
	uint32_t randomNumber = 0;
	
	/* Iterates n number of times and reads in the voltage
	   from the analog pin. Using mod 2, the lowest signficant
	   bit is computed and if it is 0 the random number is pushed
	   left by one bit, and if the LSB is 1, then the random
	   number is added by 1 and pushed to the left.
	*/
	for (int i = 0; i < n; i++) {
		voltageReading = analogRead(analogpin);
		recordLSB = voltageReading % 2;

		if (recordLSB == 0) {
			randomNumber = randomNumber << 1ul;
		} else {
			randomNumber = randomNumber << 1ul;
			randomNumber = randomNumber + 1;
		}
		//randomNumber += recordLSB*(pow(2,i));
		delay(5);
	}

	randomNumber = randomNumber + pow(2,n);

	if (randomNumber >= pow(2,n) && randomNumber < pow(2,n+1)) {
		return randomNumber;
	} else {
		generate_random_number(n);
	}

}

/*
	Description:
	Generates a number between [2^14,2^15) using generate_random_number()
	and then checks if that is prime. If the q_value is not prime then it
	keeps generating new random numbers.

	Return:
	- uint32_t p_value
*/
uint32_t get_p_val(int p_k) {
	uint32_t p_value;

	bool check = false;

	while (check != true) {
		//Serial.println("Generating numbers:");
		p_value = generate_random_number(p_k);
		//Serial.println(p_value);
		check = check_prime(p_value);
		delay(2);
	}

	return p_value;
}

/*
	Description:
	Generates a number between [2^15,2^16) using generate_random_number()
	and then checks if that is prime. If the q_value is not prime then it
	keeps generating new random numbers.

	Return:
	- uint32_t q_value
*/
uint32_t get_q_val(int q_k) {
	uint32_t q_value;

	bool check = false;

	while (check != true) {
		//Serial.println("Generating numbers:");
		q_value = generate_random_number(q_k);
		//Serial.println(q_value);
		check = check_prime(q_value);
		delay(2);
	}

	return q_value;
}

/*
	Description:
	Calculates the modulus value using mod = p*q

	Return:
	- uint32_t mod.
*/
uint32_t get_ard_mod(uint32_t p_value, uint32_t q_value) {
	uint32_t mod = (p_value * q_value);

	return mod;
}

/*
	Description:
	Calculates the phi value using totient formula

	Return:
	- uint32_t phi
*/
uint32_t get_phi(uint32_t p_value, uint32_t q_value) {
	uint32_t phi;

	phi = (p_value - 1) * (q_value - 1);

	return phi;
}

/*
	Description:
	Finds the greatest common divisor between two integers. The code was
	adopted from GCD_TEST.CPP file upload on ECLASS.

	Returns:
	- uint32_t invalidated pub_key
*/
uint32_t find_gcd(uint32_t pub_key, uint32_t phi)
{	
    while (phi > 0) {
        pub_key %= phi;
        // now swap them
        unsigned int tmp = pub_key;
        pub_key = phi;
        phi = tmp;
    }

    return pub_key;
}

/*
	Description:
	To find the public key. The function generates a random number using the
	generat_random_numer function. Then it checks if the random number has a
	greatest common divisor of 1 with the phi_value.

	Return:
		- uint32_t public key
*/
uint32_t get_public_key(uint32_t phi_value) {
	uint32_t pub_key = 0;
	uint32_t n = 0;

	// Until a valid public is not found new random numbers is generated and tested.
	while (n != 1) {
		pub_key = generate_random_number(15);
		n = find_gcd(pub_key, phi_value);
	}

	return pub_key;
}

/*
	Description:
	To find the private key, the function uses the euclidean extended algorithm that
	also handles negative and overflow outputs. With the help of the pseudocode from
	ECLASS (CMPUT 274 - ARDUINO - RSA KEY GENERATION) slides the code below was
	implemented. And in order to handle negative values we adopted the idea from the
	euclidean extended alogrithm worksheet.

	Return:
		- if x is positive and in range (o < x < phi(n)) then return x%phi(n) (also just x)
		- if x is negative then a few operations are done and the value is returned.
*/
uint32_t get_private_key(uint32_t pub_key, uint32_t phi_value, uint32_t myArd_mod) {
	// Initialize variables
	int32_t r[40], s[40], t[40];
	int32_t q, x, i;
	uint32_t return_value;

	// Initialize array index has mentioned in the slides
	r[0] = pub_key;
	s[0] = 1;
	t[0] = 0;
	r[1] = phi_value;
	s[1] = 0;
	t[1] = 1;

	// Runs the EEA.
	i = 1;
	while (r[i] > 0) {
		q = r[i-1]/r[i];
		r[i+1] = r[i-1] - q*r[i];
		s[i+1] = s[i-1] - q*s[i];
		t[i+1] = t[i-1] - q*t[i];
		i = i+1;
	}

	x = s[i-1];

	if (x < 0) {
		uint32_t z = (((-1 * x) / phi_value) + 1);
		return_value = ((x + (z*phi_value)) % phi_value);
		return return_value;
	} else {
		return_value = (x % phi_value);
		return return_value;
	}

} 

//#################################### PART 1 ##########################################################################// 

bool isServer() {
    if (digitalRead(serverPin) == HIGH) {
        return true;
    } else {
        return false;
    }
}

/*
    Compute and return (a*b)%m
    Note: m must be less than 2^31
    Arguments:
        a (uint32_t): The first multiplicant
        b (uint32_t): The second multiplicant
        m (uint32_t): The mod value
    Returns:
        result (uint32_t): (a*b)%m
*/
uint32_t multMod(uint32_t a, uint32_t b, uint32_t m) {
    uint32_t result = 0;
    uint32_t dblVal = a%m;
    uint32_t newB = b;

    // This is the result of working through the worksheet.
    // Notice the extreme similarity with powmod.
    while (newB > 0) {
        if (newB & 1) {
            result = (result + dblVal) % m;
        }
        dblVal = (dblVal << 1) % m;
        newB = (newB >> 1);
    }

    return result;
}


/*
    NOTE: This was modified using our multMod function, but is otherwise the
    function powModFast provided in the lectures.

    Compute and return (a to the power of b) mod m.
	  Example: powMod(2, 5, 13) should return 6.
*/
uint32_t powMod(uint32_t a, uint32_t b, uint32_t m) {
    uint32_t result = 1 % m;
    uint32_t sqrVal = a % m;  // stores a^{2^i} values, initially 2^{2^0}
    uint32_t newB = b;

    // See the lecture notes for a description of why this works.
    while (newB > 0) {
        if (newB & 1) {  // evalutates to true iff i'th bit of b is 1 in the i'th iteration
            result = multMod(result, sqrVal, m);
        }
        sqrVal = multMod(sqrVal, sqrVal, m);
        newB = (newB >> 1);
    }

    return result;
}



/** Writes an uint32_t to Serial3, starting from the least-significant
 * and finishing with the most significant byte.
 */
void uint32_to_serial3(uint32_t num) {
    Serial3.write((char) (num >> 0));
    Serial3.write((char) (num >> 8));
    Serial3.write((char) (num >> 16));
    Serial3.write((char) (num >> 24));
}


/** Reads an uint32_t from Serial3, starting from the least-significant
 * and finishing with the most significant byte.
 */
uint32_t uint32_from_serial3() {
    uint32_t num = 0;
    num = num | ((uint32_t) Serial3.read()) << 0;
    num = num | ((uint32_t) Serial3.read()) << 8;
    num = num | ((uint32_t) Serial3.read()) << 16;
    num = num | ((uint32_t) Serial3.read()) << 24;
    return num;
}


/*
    Encrypts using RSA encryption.

    Arguments:
        c (char): The character to be encrypted
        e (uint32_t): The partner's public key
        m (uint32_t): The partner's modulus

    Return:
        The encrypted character (uint32_t)
*/
uint32_t encrypt(char c, uint32_t e, uint32_t m) {
    return powMod(c, e, m);
}


/*
    Decrypts using RSA encryption.

    Arguments:
        x (uint32_t): The communicated integer
        d (uint32_t): The Arduino's private key
        n (uint32_t): The Arduino's modulus

    Returns:
        The decrypted character (char)
*/
char decrypt(uint32_t x, uint32_t d, uint32_t n) {
    return (char) powMod(x, d, n);
}

/*
    Core communication loop
    d, n, e, and m are according to the assignment spec
*/
void communication(uint32_t d, uint32_t n, uint32_t e, uint32_t m) {
    // Consume all early content from Serial3 to prevent garbage communication
    while (Serial3.available()) {
        Serial3.read();
    }

    // Enter the communication loop
    while (true) {
        // Check if the other Arduino sent an encrypted message.
        if (Serial3.available() >= 4) {
            // Read in the next character, decrypt it, and display it
            uint32_t read = uint32_from_serial3();
            Serial.print(decrypt(read, d, n));
        }

        // Check if the user entered a character.
        if (Serial.available() >= 1) {
            char byteRead = Serial.read();
            // Read the character that was typed, echo it to the serial monitor,
            // and then encrypt and transmit it.
            if ((int) byteRead == '\r') {
                // If the user pressed enter, we send both '\r' and '\n'
                Serial.print('\r');
                uint32_to_serial3(encrypt('\r', e, m));
                Serial.print('\n');
                uint32_to_serial3(encrypt('\n', e, m));
            } else {
                Serial.print(byteRead);
                uint32_to_serial3(encrypt(byteRead, e, m));
            }
        }
    }
}

bool wait_on_serial3( uint8_t nbytes, long timeout){
	unsigned long deadline = millis() + timeout;// wraparound not a problem
	while(Serial3.available()<nbytes && (timeout<0 || millis()<deadline)){
		delay(1);
	}

	return Serial3.available()>=nbytes;
}


//################################################################ END ############################################################//

void server_client_handshake(uint32_t private_key, uint32_t myArd_mod, uint32_t public_key, bool server) {
	uint32_t s_array[2]={0};

	if(server==true){
		// sets initial state
		ConnectionState state = LISTEN;

		while(state!=DATAEXCHANGE){
			if(state==LISTEN){
				// Listen for CR from client arduino
				if (wait_on_serial3(1,-1)) {
					if(Serial3.read()==67){
						state=WAITINGFORKEY;
					}
				}

			}else if(state==WAITINGFORKEY){
				if( wait_on_serial3(8,1000) ){
					// store skey and smod are what we recive from other arduino
					s_array[0] = uint32_from_serial3();
					s_array[1] = uint32_from_serial3();

					// acknowledge and send server arduinos public key/mod to client arduino
					Serial3.write('A');
					uint32_to_serial3(public_key);
					uint32_to_serial3(myArd_mod);
					state=WAITINGFORACK;

				}else{
					//timeout return to LISTEN state
					state=LISTEN;
				}

			}else if(state==WAITINGFORACK){
				if(wait_on_serial3(1,1000)){
					char data = Serial3.read();				
					if (data=='A'){
						state=DATAEXCHANGE;
					}else if (data=='C'){
						state=WAITINGFORKEY2;
					}else{
						state = LISTEN;
					}

				}else{
					state=LISTEN;
				}

			}else if(state==WAITINGFORKEY2){
				if( wait_on_serial3(8,1000) ){
					// store skey and smod that we recieve from client arduino
					s_array[0] = uint32_from_serial3();
					s_array[1] = uint32_from_serial3();
					state=WAITINGFORACK;

				}else{
					//timeout return to LISTEN state
					state=LISTEN;
				}
			}
		}// end of while loop

	
	//code for client arduino
	}else{
		// sets inital state
		ConnectionState state = START;

		while(state!=DATAEXCHANGE){// state!=dataexchange
			if(state == START){
				// send connection request CR(ckey,mod) 9 bytes
				Serial3.write('C');//write('C')
				uint32_to_serial3(public_key);
				uint32_to_serial3(myArd_mod);

				// change state to wait for acknowledge
				state=WAITINGFORACK;

			}else if(state== WAITINGFORACK){
				if( wait_on_serial3(9,1000) ){
					if(Serial3.read() == 65){
						// store skey, smod
						s_array[0] = uint32_from_serial3();
						s_array[1] = uint32_from_serial3();

						// clear Serial3 buffer 
						while(Serial3.available() >0){
							Serial3.read();
						}
					
						Serial3.write('A');
						state = DATAEXCHANGE;

					}else{
						// timeout and change state back to START
						state = START;
					}	

				}else{
					// timeout and change state back to START
					state = START;
				}
			}
		}// end of while loop	
	}// end of else

    Serial.println("Handshake complete, ready to communicate");

    // Now enter the communication phase.
    communication(private_key, myArd_mod, s_array[0], s_array[1]);

}// end of server_client_handshake()


int main() {
	setup();
	bool server;
    server = isServer();// server = true then arudino is server else vice versa

	uint32_t p_value = 0;
	uint32_t q_value = 0;
	uint32_t myArd_mod = 0;
	uint32_t phi_value = 0;
	uint32_t public_key = 0;
	uint32_t private_key = 0;
	int p_k = 14;
	int q_k = 15;
	
	p_value = get_p_val(p_k);
	q_value = get_q_val(q_k);

	myArd_mod = get_ard_mod(p_value, q_value);
	Serial.print("This arduino's mod: ");
	Serial.println(myArd_mod);

	phi_value = get_phi(p_value, q_value);

	public_key = get_public_key(phi_value);
	Serial.print("Public key: ");
	Serial.println(public_key);

	private_key = get_private_key(public_key, phi_value, myArd_mod);
	Serial.print("Private key: ");
	Serial.println(private_key);

	server_client_handshake(private_key, myArd_mod, public_key, server);

}// end of int main()