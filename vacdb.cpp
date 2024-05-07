// CMSC 341 - Spring 2024 - Project 4

/* vaprosi
 - prover esli destructor rabotayet 
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
    m_newPolicy = m_currProbing;

    m_currentTable = new Patient * [m_currentCap] ();
    m_oldTable = nullptr;
    
    m_currentSize = 0;
    m_currNumDeleted = 0;
    
    m_oldCap = m_oldSize = m_oldNumDeleted = 0;

}

VacDB::~VacDB(){
    if (m_currentSize != 0) 
        clear(m_currentTable, m_currentCap);
}

void VacDB::clear(Patient ** table, int size) {
    // Deallocate objects
    for (int i = 0; i < size; ++i) {
        if (table[i]) {
            delete table[i];
            table[i] = nullptr; // Set to null to avoid dangling pointers (optional)
        }
    }

    // Deallocate pointer array
    delete[] table;
    table = nullptr; // Set to null to avoid dangling pointers (optional)
}

// changeProbPolicy(prob_t policy)
// Stores the new policy and implements it next time the table is reHashed
void VacDB::changeProbPolicy(prob_t policy){
    m_newPolicy = policy;
}

// insert(Patient patient)
// Calculates 
bool VacDB::insert(Patient patient){
    unsigned int index = m_hash(patient.m_name) % m_currentCap;

    

    Patient *newPatient = new Patient(patient);

    // if the index is free, we insert the newPatient
    if (m_currentTable[index] == nullptr) {
        m_currentTable[index] = newPatient;
    }
    // else there is a collision and it needs to probe
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
                int newIndex = ((m_hash(patient.m_name) % m_currentCap) + i * (11 - (m_hash(patient.m_name) % 11))) % m_currentCap;
                i++;
                index = newIndex;
            }

            m_currentTable[index] = newPatient;

        }
    }
    m_currentSize++;


    // checking Load Factor
    if (lambda() > 0.50 || deletedRatio() > 0.80) {
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

    // checks if there is a new probing policy
    m_currProbing = (m_currProbing != m_newPolicy) ? m_newPolicy : m_currProbing;

    // iterates four times, inserts a quarter at a time
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

// copyCurrent() helper function
// copies all the data in current variables to old variables
void VacDB::copyCurrent() {
    m_oldTable = m_currentTable;
    m_oldCap = m_currentCap;
    m_oldNumDeleted = m_currNumDeleted;
    m_oldSize = m_currentSize;
    m_oldProbing = m_currProbing;
}


bool VacDB::remove(Patient patient){
    unsigned int index = m_hash(patient.m_name) % m_currentCap;
    int iter = 0;

    // Iterates 
    while (iter < m_currentCap) {
        if (m_currentTable[index]) {

            // if table[index] matches the patient we're looking for
            if (m_currentTable[index]->getKey() == patient.getKey() && m_currentTable[index]->getSerial() == patient.getSerial()) {
                
                // if Patient is already marked as deleted
                if (!(m_currentTable[index]->m_used))
                    return false;
                
                // else we set it as deleted
                m_currentTable[index]->setUsed(false);
                m_currNumDeleted++;

                // check
                cout << "lambda: " << lambda() << " deletedRatio: " << deletedRatio() << "\n";
                
                if (lambda() > 0.50 || deletedRatio() > 0.80) {
                    copyCurrent(); // copies all current variable to old
                    m_currentCap = findNextPrime((m_oldSize - m_oldNumDeleted) * 4);
        
                    m_currentTable = new Patient * [m_currentCap];
                    m_currentSize = 0;

                    reHash();
                }

                return true;
            }

            // if table[index] doesnt match then we probe
            else if (m_currProbing == LINEAR) {
                index = (index + 1) % m_currentCap;
            }
            else if (m_currProbing == QUADRATIC) {
                index = (index + (iter * iter)) % m_currentCap;
            }
            else if (m_currProbing == DOUBLEHASH) {
                index = ((m_hash(patient.getKey()) % m_currentCap) + iter * (11 - (m_hash(patient.getKey()) % 11))) % m_currentCap;
            }
        }
        // patient doesnt exist
        else {
            return false;
        }
        iter++;
    }

    // Doesnt exist
    return false;
}

// const getPatient(string name, int serial) const
// based on the type of probing, it takes hashes through the hash table until it finds the hash
const Patient VacDB::getPatient(string name, int serial) const{
    unsigned int index = m_hash(name) % m_currentCap;
    int iter = 0;

    while (iter < m_currentCap) {
        if (m_currentTable[index]) {
            if (m_currentTable[index]->m_name == name && m_currentTable[index]->m_serial == serial) {
                return *m_currentTable[index];
            }
            else if (m_currProbing == LINEAR) {
                index = (index + 1) % m_currentCap;
            }
            else if (m_currProbing == QUADRATIC) {
                index = (index + (iter * iter)) % m_currentCap;
            }
            else if (m_currProbing == DOUBLEHASH) {
                index = ((m_hash(name) % m_currentCap) + iter * (11 - (m_hash(name) % 11))) % m_currentCap;
            }
        }
        else {
            Patient emptyPatient = Patient();
            return emptyPatient;
        }
        iter++;
    }

    // Doesnt exist
    Patient emptyPatient = Patient();
    return emptyPatient;
    
}

// updateSerialNumber(Patient patient, int serial)
// Changes the serial of the patient
bool VacDB::updateSerialNumber(Patient patient, int serial){
    unsigned int index = m_hash(patient.m_name) % m_currentCap;
    int iter = 0;

    // Iterates 
    while (iter < m_currentCap) {
        if (m_currentTable[index]) {
            if (m_currentTable[index]->getKey() == patient.getKey() && m_currentTable[index]->getSerial() == patient.getSerial()) {
                
                m_currentTable[index]->setSerial(serial);
                return true;
            }
            else if (m_currProbing == LINEAR) {
                index = (index + 1) % m_currentCap;
            }
            else if (m_currProbing == QUADRATIC) {
                index = (index + (iter * iter)) % m_currentCap;
            }
            else if (m_currProbing == DOUBLEHASH) {
                index = ((m_hash(patient.getKey()) % m_currentCap) + iter * (11 - (m_hash(patient.getKey()) % 11))) % m_currentCap;
            }
        }
        else {
            return false;
        }
        iter++;
    }

    // Doesnt exist
    return false;
    
}

// lambda()
// returns the size to cap ratio
float VacDB::lambda() const {
    return ((float)m_currentSize / (float)m_currentCap);
}

// deletedRatio()
// returns the deleted to size ratio
float VacDB::deletedRatio() const {
    if (m_currentSize == 0) {
        return 0.0;
    }
    return ((float) m_currNumDeleted / (float) m_currentSize);
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
