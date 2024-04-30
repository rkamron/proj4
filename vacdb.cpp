// CMSC 341 - Spring 2024 - Project 4

/* shit to do
 - when size isnt a prime in constructor
 - 

*/

#include "vacdb.h"
VacDB::VacDB(int size, hash_fn hash, prob_t probing = DEFPOLCY){
    
    // conditions check if size is valid
    if (MINPRIME >= size) m_currentCap = MINPRIME;
    else if (MAXPRIME <= size) m_currentCap = MAXPRIME;
    else m_currentCap = (isPrime(size)) ? size : findNextPrime(size);

    m_hash = hash;
    m_currProbing = probing;

    m_currentTable = new Patient * [m_currentCap];
    m_oldTable = nullptr;
    
    m_currentSize = 0;
    m_currNumDeleted = 0;
    
    m_oldCap = m_oldSize = m_oldNumDeleted = 0;

}

VacDB::~VacDB(){
    
}

void VacDB::changeProbPolicy(prob_t policy){
    
}

bool VacDB::insert(Patient patient){
    unsigned int index = m_hash(patient.m_name) % m_currentCap;

    Patient *newPatient = new Patient(patient);

    if (m_currentTable[index] == nullptr) {
        m_currentTable[index] = newPatient;
    }
    else {
        if (m_currProbing == LINEAR) {
            while (m_currentTable[index] != nullptr) {
                index++;
            }
            
            m_currentTable[index] = newPatient;

        }
        else if (m_currProbing == QUADRATIC) {
            int i = 0;
            while (m_currentTable[index]) {
                index += i*i;
                i++;
            }
        }
        else if (m_currProbing == DOUBLEHASH) {
            int i = 0;
            while (m_currentTable[index]) {
                int newIndex = ((m_hash(patient.m_name) % m_currentCap) + i * (11 - (m_hash(patient.m_name)) % 11)) % m_currentSize;
                cout << newIndex << "\n";
                i++;
            }

            m_currentTable[index] = newPatient;

        }
    }
    m_currentSize++;
    cout << (float)m_currentSize << " " << (float)m_currentCap << "\n";
    // checking Load Factor
    if (((float)m_currentSize / (float)m_currentCap) > 0.50) {
        reHash();
        copyCurrent(); // copies all current variable to old
        m_currentCap = findNextPrime(m_oldSize * 4);


    }

    return true;

}

void VacDB::reHash() {
    cout << "REHASHING\n";
    
    m_transferIndex = 0;

    int quarter = m_oldCap / 4;
    for (int i = 0; i < 4; i++) {
        for (int j = m_transferIndex; j < i + m_oldCap/4; j++) {
            
            if (m_oldTable[j] != nullptr) {
                insert(*m_oldTable[j]);

            }
            
            m_transferIndex++;
        }
    }

}


void VacDB::copyCurrent() {
    m_oldTable = m_currentTable;
    m_oldCap = m_currentCap;
    m_oldNumDeleted = m_currNumDeleted;
    m_oldSize = m_currentSize;
    m_oldProbing = m_currProbing;
}

bool VacDB::remove(Patient patient){
    int index = m_hash(patient.m_name) % m_currentCap;


}

const Patient VacDB::getPatient(string name, int serial) const{
    unsigned int index = m_hash(name) % m_currentCap;
    cout << m_currentTable[index] << index << "\n";
    //cout << "Name: " << name << ", Index:\t" << index << "\n";
    int patSerial = m_currentTable[index]->m_serial;
    
    // else we have to continue searching through the hash table
    if (m_currProbing == LINEAR) {
        while (m_currentTable[index]->getSerial() != serial) {
            index++;
        }
    }
    else if (m_currProbing == QUADRATIC) {
        int i = 0;
        while (m_currentTable[index]->getSerial() != serial) {
            index += i*i;
            i++;
        }
    }
    /*
    else if (m_currProbing == DOUBLEHASH) {
        int i = 0;
        while (m_currentTable[index]->getSerial() != serial) {
            int newIndex = ((m_hash(patient.m_name) % m_currentCap) + i * (11 - (m_hash(patient.m_name)) % 11)) % m_currentSize;
            cout << newIndex << "\n";
            i++;
        }

        m_currentTable[index] = newPatient;

    }
    */

   return *m_currentTable[index];
}

bool VacDB::updateSerialNumber(Patient patient, int serial){
    
}

float VacDB::lambda() const {
    
}

float VacDB::deletedRatio() const {
    
}

void VacDB::dump() const {
    cout << "Dump for the current table: " << endl;
    if (m_currentTable != nullptr)
        for (int i = 0; i < m_currentCap; i++) {
            cout << "[" << i << "] : " << m_currentTable[i] << endl;
        }
    cout << "Dump for the old table: " << endl;
    if (m_oldTable != nullptr)
        for (int i = 0; i < m_oldCap; i++) {
            cout << "[" << i << "] : " << m_oldTable[i] << endl;
        }
}

bool VacDB::isPrime(int number){
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

int VacDB::findNextPrime(int current){
    //we always stay within the range [MINPRIME-MAXPRIME]
    //the smallest prime starts at MINPRIME
    if (current < MINPRIME) current = MINPRIME-1;
    for (int i=current; i<MAXPRIME; i++) { 
        for (int j=2; j*j<=i; j++) {
            if (i % j == 0) 
                break;
            else if (j+1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    //if a user tries to go over MAXPRIME
    return MAXPRIME;
}

ostream& operator<<(ostream& sout, const Patient* patient ) {
    if ((patient != nullptr) && !(patient->getKey().empty()))
        sout << patient->getKey() << " (" << patient->getSerial() << ", "<< patient->getUsed() <<  ")";
    else
        sout << "";
  return sout;
}

bool operator==(const Patient& lhs, const Patient& rhs){
    // since the uniqueness of an object is defined by name and serial number
    // the equality operator considers only those two criteria
    return ((lhs.getKey() == rhs.getKey()) && (lhs.getSerial() == rhs.getSerial()));
}

bool Patient::operator==(const Patient* & rhs){
    // since the uniqueness of an object is defined by name and serial number
    // the equality operator considers only those two criteria
    return ((getKey() == rhs->getKey()) && (getSerial() == rhs->getSerial()));
}
