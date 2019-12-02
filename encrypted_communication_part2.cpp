#include <Arduino.h>
#include <math.h>

const int analogpin = 1;
const int config_pin = 13;
enum ConnectionState {
  	START, WAITINGFORACK, DATAEXCHANGE, LISTEN, WAITINGFORKEY
};


void setup() {
	init();
	Serial.begin(9600);
	Serial3.begin(9600);
	pinMode(analogpin, INPUT);
	pinMode(config_pin, INPUT);
}


bool display_arduino_type()
{
    bool server;

    if (digitalRead(config_pin) == HIGH) {
        //bool server = true;
        return true;
        Serial.println("Server Arduino");
    }
    else {
        //bool server = false;
        return false;
        Serial.println("Client Arduino");
    }
    //Serial.flush();
    return server;
}


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


uint32_t generate_random_number(int n) {
	int voltageReading;
	int recordLSB = 0;
	uint32_t randomNumber = 0;
	
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

	//return 0;
}


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


uint32_t get_ard_mod(uint32_t p_value, uint32_t q_value) {
	uint32_t mod = (p_value * q_value);

	return mod;
}


uint32_t get_phi(uint32_t p_value, uint32_t q_value) {
	uint32_t phi;

	phi = (p_value - 1) * (q_value - 1);

	return phi;
}


unsigned int find_gcd(uint32_t pub_key, uint32_t phi)
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


uint32_t get_public_key(uint32_t phi_value) {
	uint32_t pub_key = 0;
	uint32_t n = 0;

	while (n != 1) {
		pub_key = generate_random_number(15);
		n = find_gcd(pub_key, phi_value);
	}

	return pub_key;
}


uint32_t get_private_key(uint32_t pub_key, uint32_t phi_value, uint32_t myArd_mod) {
	int32_t r[40], s[40], t[40];
	int32_t q, x, i;
	uint32_t return_value;
	r[0] = pub_key;
	s[0] = 1;
	t[0] = 0;
	r[1] = phi_value;
	s[1] = 0;
	t[1] = 1;

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
		//Serial.println("x < 0");
		uint32_t z = (((-1 * x) / phi_value) + 1);
		return_value = ((x + (z*phi_value)) % phi_value);
		if (return_value < 0 || return_value >= phi_value) {
			Serial.println("Private_key domain error");
		}
		return return_value;
	} else {
		//Serial.println("x >= 0");
		return_value = (x % phi_value);
		if (return_value < 0 || return_value >= phi_value) {
			Serial.println("Private_key domain error");
		}
		return return_value;
	}

} 












uint32_t encrypt(uint32_t c, uint32_t e, uint32_t n)
{
    uint32_t ans = 0;
    uint32_t pow_x = c;

    // To prevent overflow we can use repeated multipication
    while (e > 0) {
        // e is odd then add pow_x to ans
        if ((e % 2) == 1) {
            ans = (ans + pow_x) % n;
        }

        // Multiply pow_x by 2 and then divide e by 2
        pow_x = (pow_x * 2) % n;
        e <<= 1;
    }

    return ans;
}

/* 
  Description:
  Takes in a character from server() or client() through serial3 and performs 
  modular arthemtic to decrypt the character into a uint32_t variable 
  and returns it back to server() or client()

  Arguments:
  uint32_t x = input byte from serial3
  uint32_t d = Client's/Server's private key
  uint32_t m = Client's/Server's modulus
*/
uint32_t decrypt(uint32_t x, uint32_t d, uint32_t m)
{
    uint32_t ans = 0;
    uint32_t pow_x = x;

    // To prevent overflow we can use repeated multipication
    while (d > 0) {
        // e is odd then add pow_x to ans
        if ((d % 2) == 1) {
            ans = (ans + pow_x) % m;
        }

        // Multiply pow_x by 2 and then divide e by 2
        pow_x = (pow_x * 2) % m;
        d <<= 1;
    }
    return ans;
}

/* 
  Description:
  Sends a byte at a time to serial3, due to the way characters
  are encrypted in RSA

  Arguments:
  uint32_t num = encrypted character to be sent
*/
void uint32_to_serial3(uint32_t num)
{
	Serial3.flush();
    Serial3.write((char)(num >> 0));
    Serial3.write((char)(num >> 8));
    Serial3.write((char)(num >> 16));
    Serial3.write((char)(num >> 24));
}

/* 
  Description:
  Reads a byte at a time from serial3, due to the way characters
  are encrypted in RSA
*/
uint32_t uint32_from_serial3()
{
    uint32_t num = 0;
    num = num | ((uint32_t)Serial3.read()) << 0;
    num = num | ((uint32_t)Serial3.read()) << 8;
    num = num | ((uint32_t)Serial3.read()) << 16;
    num = num | ((uint32_t)Serial3.read()) << 24;
    // sends num to server() or client() functions
    return num;
}

/* 
  Description:
  Declares the appropiate keys to the variables d, n, e and m.
  Main purpose of thhis function is facilitate sending/recieving data
  functions, encrypt/decrypt functions and write characters back onto
  serial. The client() and server() function are analgous in their program
  operation however, the main difference is the declaration of keys (public,
  private and modulus)
*/
void server_arduino(uint32_t d, uint32_t n, uint32_t e, uint32_t m)
{
	//Serial.println('hello we in server_arduino');
    // if there is a byte to read, read it and send to Arduino B
    //uint32_t d = ServerPrivateKey;
    //uint32_t n = ServerModulus;
    //uint32_t e = ClientPublicKey;
    //uint32_t m = ClientModulus;
    if (Serial.available() > 0) {
        uint32_t byte_read = Serial.read(); // ASCII value of key pressed
        Serial.write(byte_read);
        Serial.flush();
        uint32_t encrypt_ans = encrypt(byte_read, e, n);
        uint32_to_serial3(encrypt_ans);

        if (byte_read == 13) {
            byte_read = 10;
            Serial.write(byte_read);
            Serial.flush();
            uint32_t encrypt_ans = encrypt(byte_read, e, n);
            uint32_to_serial3(encrypt_ans);
        }
    }
    if (Serial3.available() > 3) {
        uint32_t byte_read = uint32_from_serial3();
        uint32_t decrypt_ans = decrypt(byte_read, d, m);
        Serial.write(decrypt_ans);
        Serial.flush();
    }
}

/* 
  Description:
  Declares the appropiate keys to the variables d, n, e and m.
  Main purpose of thhis function is facilitate sending/recieving data
  functions, encrypt/decrypt functions and write characters back onto
  serial. The client() and server() function are analgous in their program
  operation however, the main difference is the declaration of keys (public,
  private and modulus)
*/
void client_arduino(uint32_t d, uint32_t n, uint32_t e, uint32_t m)
{
    //uint32_t d = ClientPrivateKey;
    //uint32_t n = ClientModulus;
    //uint32_t e = ServerPublicKey;
    //uint32_t m = ServerModulus;
    if (Serial.available() > 0) {
        uint32_t byte_read = Serial.read();
        Serial.write(byte_read);
        Serial.flush();
        uint32_t encrypt_ans = encrypt(byte_read, e, n);
        uint32_to_serial3(encrypt_ans);

        if (byte_read == 13) {
            byte_read = 10;
            Serial.write(byte_read);
            Serial.flush();
            uint32_t encrypt_ans = encrypt(byte_read, e, n);
            uint32_to_serial3(encrypt_ans);
        }
    }
    if (Serial3.available() > 3) {
        uint32_t byte_read = uint32_from_serial3();
        uint32_t decrypt_ans = decrypt(byte_read, d, m);
        Serial.write(decrypt_ans);
        Serial.flush();
    }
}


bool wait_on_serial3( uint8_t nbytes, long timeout){
	unsigned long deadline = millis() + timeout;// wraparound not a problem
	while(Serial3.available()<nbytes && (timeout<0 || millis()<deadline)){
		delay(1);
	}

	return Serial3.available()>=nbytes;
}










int main() {
	setup();
	bool server;
    server = display_arduino_type();// server = true then arudino is server else vice versa
    if(server==true){
    	Serial.println("is tru");
    }else{
    	Serial.println("is fals");
    }
    //Serial.println(server);
	uint32_t p_value = 0;
	uint32_t q_value = 0;
	uint32_t myArd_mod = 0;
	uint32_t phi_value = 0;
	uint32_t public_key = 0;
	uint32_t private_key = 0;
	uint32_t s_array[2]={0};
	int p_k = 14;
	int q_k = 15;
	
	//for (int i = 0; i < 30; i++) {
	p_value = get_p_val(p_k);
	//Serial.print("p value = ");
	//Serial.println(p_value);

	q_value = get_q_val(q_k);
	//Serial.print("q value = ");
	//Serial.println(q_value);

	myArd_mod = get_ard_mod(p_value, q_value);
	Serial.print("This arduino's mod: ");
	Serial.println(myArd_mod);

	phi_value = get_phi(p_value, q_value);
	//Serial.print("Phi value: ");
	//Serial.println(phi_value);

	public_key = get_public_key(phi_value);
	Serial.print("Public key: ");
	Serial.println(public_key);

	private_key = get_private_key(public_key, phi_value, myArd_mod);
	Serial.print("Private key: ");
	Serial.println(private_key);

	Serial.flush();

	if (p_value < pow(2,14) || (p_value >= pow(2,15))) {
		Serial.println("p_value error");
	}

	if (q_value < pow(2,15) || (p_value >= pow(2,16))) {
		Serial.println("q_value error");
	}

	if (myArd_mod < pow(2,29) || (myArd_mod >= pow(2,31))) {
		Serial.println("mod_value error");
	}

	if (public_key <= 1 || public_key >= phi_value) {
		Serial.println("public_key not domain");
	}

	//} end of testing for loop
	
	

	Serial.println(server);
	// code for server arduino
	if(server==true){
		Serial.println("In server==true");
		// sets initial state
		ConnectionState state = LISTEN;

		while(state!=DATAEXCHANGE){
			if(state==LISTEN){
				//Serial.println("Inside state==LISTEN");
				//char data = Serial3.read(); date=='C'
				//Serial.println(Serial3.read());
				if(Serial3.read()==67){// Serial3.read() reads int int ascii value
					state=WAITINGFORKEY;
					Serial.println("in LISTEN state");
				}
			}else if(state==WAITINGFORKEY){
				Serial.println("in WAITINGFORKEY state");
				if( wait_on_serial3(8,1000) ){
					//skey and smod are what we recive from other arduino
					s_array[0] = uint32_from_serial3();
					s_array[1] = uint32_from_serial3();

					// acknowledge and send this arduinos public key/mod to other arduino
					Serial3.write('A');
					uint32_to_serial3(public_key);
					uint32_to_serial3(myArd_mod);
					state=WAITINGFORACK;// added in
				}else{
					//timeout return to LISTEN state
					state=LISTEN;
				}
			}else if(state==WAITINGFORACK){
				Serial.println("in WAITINGFORACK state");
				if(wait_on_serial3(1,1000)){
					if(Serial3.read()==65){
						state=DATAEXCHANGE;
						Serial.println('now at dataexchange');
						Serial.flush();
						
					}else if(Serial3.read()==67){
						state=WAITINGFORKEY;
					}
				}else{
					state=LISTEN;
				}
			}
		}// end of while loop

	//code for client arduino
	Serial.println('handshake complete for server');
	}else{
		// sets inital state
		ConnectionState state = START;

		while(state!=DATAEXCHANGE){// state!=dataexchange
			if(state == START){
				Serial.println("in START state");
				//Serial.write(67);
				// send connection request CR(ckey,mod) 9 bytes
				Serial3.write(67);//write('C')
				uint32_to_serial3(public_key);
				uint32_to_serial3(myArd_mod);

				//change state to wait for acknowledge
				state=WAITINGFORACK;

			}else if(state== WAITINGFORACK){
				Serial.println("in WAITINGFORACK state");
				if( wait_on_serial3(9,1000) ){
					if(Serial3.read()==65){
						// store skey, smod
						s_array[0] = uint32_from_serial3();
						s_array[1] = uint32_from_serial3();

						// send acknowledge to server
						Serial3.write('A');
						state = DATAEXCHANGE;


					}else{
						state = START;
					}	

				}else{
					// timeout and change state back to Start
					state = START;
				}
			}
		}// end of while loop	
	}// end of else 


	Serial.println("before infinte while loop");// not reaching here
	//now run infinte while loop so arduinos can communicate
	/*
	while (true) {
        if (server == true) {
            server_arduino(private_key, myArd_mod, s_array[0], s_array[1]);
        }
        else {
            client_arduino(private_key, myArd_mod, s_array[0], s_array[1]);
        }
    }
	*/
	delay(1000);
	return 0;
}

	/* Testing sequences
	if (p_value < pow(2,14) || (p_value >= pow(2,15))) {
		Serial.println("p_value error");
	}

	if (q_value < pow(2,15) || (p_value >= pow(2,16))) {
		Serial.println("q_value error");
	}

	if (myArd_mod < pow(2,29) || (myArd_mod >= pow(2,31))) {
		Serial.println("mod_value error");
	}

	//Serial.println(" ");

	//}
	*/
