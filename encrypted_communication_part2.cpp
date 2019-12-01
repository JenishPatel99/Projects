#include <Arduino.h>
#include <math.h>

const int analogpin = 1;

void setup() {
	init();
	Serial.begin(9600);
	pinMode(analogpin, INPUT);

}

bool check_prime(uint32_t n) {
    bool check = true;

    for (uint32_t i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) {
            check = false;
            break;
        }
    }

    if (check == true && n != 1) {
        //Serial.println("prime");
        return true;
    } else {
        //Serial.println("not prime");
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

    Serial.println(pub_key);
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

int main() {
	setup();
	uint32_t p_value = 0;
	uint32_t q_value = 0;
	uint32_t myArd_mod = 0;
	uint32_t phi_value = 0;
	uint32_t public_key = 0;
	uint32_t private_key = 0;
	int p_k = 14;
	int q_k = 15;

	//for (int i = 0; i < 10; i++) {
	p_value = get_p_val(p_k);
	Serial.print("p value = ");
	Serial.println(p_value);

	q_value = get_q_val(q_k);
	Serial.print("q value = ");
	Serial.println(q_value);

	myArd_mod = get_ard_mod(p_value, q_value);
	Serial.print("This arduino's mod: ");
	Serial.println(myArd_mod);

	phi_value = get_phi(p_value, q_value);
	Serial.print("Phi value: ");
	Serial.println(phi_value);

	public_key = get_public_key(phi_value);
	Serial.print("Public key: ");
	Serial.println(public_key);

	private_key = get_private_key(pub_key)
	Serial.print("Private key: ");
	Serial.println(private_key);

	Serial.flush();

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