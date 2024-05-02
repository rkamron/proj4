// CMSC 341 - Spring 2024 - Project 4

/* vaprosi
 - getPatient() case when the patient doesnt exist.
 - how to test if theyre in the right buckets

*/

#include "vacdb.h"
VacDB::VacDB(int size, hash_fn hash, prob_t probing = DEFPOLCY){
    
    // conditions check if size is valid
    if (MINPRIME >= size) m_currentCap = MINPRIME;
    else if (MAXPRIME <= size) m_currentCap = MAXPRIME;
    else m_currentCap = (isPrime(size)) ? size : findNextPrime(size);

    m_hash = hash;
    m_currProbing = probing;

    m_currentTable = new Patient * [m_currentCap] ();
    m_oldTable = nullptr;
    
    m_currentSize = 0;
    m_currNumDeleted = 0;
    
    m_oldCap = m_oldSize = m_oldNumDeleted = 0;

}

VacDB::~VacDB(){
    clear(m_currentTable, m_currentCap);
}

void VacDB::clear(Patient ** table, int size) {
    // Deallocate objects
    for (int i = 0; i < size; ++i) {
        if (table[i] != nullptr) {
            delete table[i];
            table[i] = nullptr; // Set to null to avoid dangling pointers (optional)
        }
    }

    // Deallocate pointer array
    delete[] table;
    table = nullptr; // Set to null to avoid dangling pointers (optional)
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
                index = (index +  1) % m_currentCap;
            }
            
            m_currentTable[index] = newPatient;

        }
        else if (m_currProbing == QUADRATIC) {
            int i = 0;
            while (m_currentTable[index] != nullptr) {
                index = (index + (i*i)) % m_currentCap;
                i++;
            }
            m_currentTable[index] = newPatient;
        }
        else if (m_currProbing == DOUBLEHASH) {
            int i = 0;
            while (m_currentTable[index]) {
                int newIndex = ((m_hash(patient.m_name) % m_currentCap) + i * (11 - (m_hash(patient.m_name)) % 11)) % m_currentSize;
                i++;
                index = newIndex;
            }

            m_currentTable[index] = newPatient;

        }
    }
    m_currentSize++;


    // checking Load Factor
    if (((float)m_currentSize / (float)m_currentCap) > 0.50) {
        copyCurrent(); // copies all current variable to old
        m_currentCap = findNextPrime(m_oldSize * 4);
        
        m_currentTable = new Patient * [m_currentCap];
        m_currentSize = 0;


        reHash();
    }

    return true;

}

// reHash()
// Preconditions:   old Table and variables must already be copied over
//                  current table and variables must be ready
void VacDB::reHash() {
    m_transferIndex = 0;

    
    for (int i = 0; i < 4; i++) {
        int quarter = m_transferIndex + (m_oldCap / 4);
        while (m_transferIndex < quarter) {
            
            if (m_oldTable[m_transferIndex] != nullptr) insert(*m_oldTable[m_transferIndex]);

            m_transferIndex++;
        }
    }

    while (m_transferIndex < m_oldCap) {
        if (m_oldTable[m_transferIndex] != nullptr) insert(*m_oldTable[m_transferIndex]);
        m_transferIndex++;
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

    return false;
}

// const getPatient(string name, int serial) const
// based on the type of probing, it takes hashes through the hash table until it finds the hash
const Patient VacDB::getPatient(string name, int serial) const{
    unsigned int index = m_hash(name) % m_currentCap;
    
    // else we have to continue searching through the hash table
    if (m_currProbing == LINEAR) {
        bool flag = true;
        while (flag) {
            if (m_currentTable[index]) {
                if (m_currentTable[index]->getSerial() == serial) {
                    flag = false;
                } 
                else {
                    index = (index + 1) % m_currentCap;
                }
            } 
            else {
                index = (index + 1) % m_currentCap;
            }
        }
    }
    else if (m_currProbing == QUADRATIC) {
        int i = 0; // quadratic multiplier 
        bool flag = true; // controls while loop

        // loops through the list until it reaches the right index
        while (flag) {
            // if m_currentTable[index] exists
            if (m_currentTable[index]) {
                // if a patient exists, it checks the serial
                if (m_currentTable[index]->getSerial() == serial) {
                    flag = false;
                } 
                
                else {
                    index = (index + (i*i)) % m_currentCap;
                    i++;
                }
            }
            // else patient at index doesnt exists and we iterate
            else {
                index = (index + (i*i)) % m_currentCap;
                i++;
            }
        }
    }
    
    else if (m_currProbing == DOUBLEHASH) {
        int i = 0;
        while (m_currentTable[index]->getSerial() != serial) {
            Patient *patient = m_currentTable[index];
            int newIndex = ((m_hash(patient->m_name) % m_currentCap) + i * (11 - (m_hash(patient->m_name)) % 11)) % m_currentSize;
            cout << newIndex << "\n";
            i++;
        }


    }
    

    if (index >= m_currentCap) {
        Patient empty;
        return empty;
    }
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
