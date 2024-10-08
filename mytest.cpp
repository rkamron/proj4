// CMSC 341 - Spring 2024 - Project 4
#include "vacdb.cpp"
#include <math.h>
#include <random>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>     //used to get the current time
// We can use the Random class to generate the test data randomly!
enum RANDOM {UNIFORMINT, UNIFORMREAL, NORMAL, SHUFFLE};
class Random {
public:
    Random(int min, int max, RANDOM type=UNIFORMINT, int mean=50, int stdev=20) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            //the case of NORMAL to generate integer numbers with normal distribution
            m_generator = std::mt19937(m_device());
            //the data set will have the mean of 50 (default) and standard deviation of 20 (default)
            //the mean and standard deviation can change by passing new values to constructor 
            m_normdist = std::normal_distribution<>(mean,stdev);
        }
        else if (type == UNIFORMINT) {
            //the case of UNIFORMINT to generate integer numbers
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_unidist = std::uniform_int_distribution<>(min,max);
        }
        else if (type == UNIFORMREAL) { //the case of UNIFORMREAL to generate real numbers
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_uniReal = std::uniform_real_distribution<double>((double)min,(double)max);
        }
        else { //the case of SHUFFLE to generate every number only once
            m_generator = std::mt19937(m_device());
        }
    }
    void setSeed(int seedNum){
        // we have set a default value for seed in constructor
        // we can change the seed by calling this function after constructor call
        // this gives us more randomness
        m_generator = std::mt19937(seedNum);
    }

    void getShuffle(vector<int> & array){
        // the user program creates the vector param and passes here
        // here we populate the vector using m_min and m_max
        for (int i = m_min; i<=m_max; i++){
            array.push_back(i);
        }
        shuffle(array.begin(),array.end(),m_generator);
    }

    void getShuffle(int array[]){
        // the param array must be of the size (m_max-m_min+1)
        // the user program creates the array and pass it here
        vector<int> temp;
        for (int i = m_min; i<=m_max; i++){
            temp.push_back(i);
        }
        std::shuffle(temp.begin(), temp.end(), m_generator);
        vector<int>::iterator it;
        int i = 0;
        for (it=temp.begin(); it != temp.end(); it++){
            array[i] = *it;
            i++;
        }
    }

    int getRandNum(){
        // this function returns integer numbers
        // the object must have been initialized to generate integers
        int result = 0;
        if(m_type == NORMAL){
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else if (m_type == UNIFORMINT){
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }

    double getRealRandNum(){
        // this function returns real numbers
        // the object must have been initialized to generate real numbers
        double result = m_uniReal(m_generator);
        // a trick to return numbers only with two deciaml points
        // for example if result is 15.0378, function returns 15.03
        // to round up we can use ceil function instead of floor
        result = std::floor(result*100.0)/100.0;
        return result;
    }

    string getRandString(int size){
        // the parameter size specifies the length of string we ask for
        // to use ASCII char the number range in constructor must be set to 97 - 122
        // and the Random type must be UNIFORMINT (it is default in constructor)
        string output = "";
        for (int i=0;i<size;i++){
            output = output + (char)getRandNum();
        }
        return output;
    }
    
    private:
    int m_min;
    int m_max;
    RANDOM m_type;
    std::random_device m_device;
    std::mt19937 m_generator;
    std::normal_distribution<> m_normdist;//normal distribution
    std::uniform_int_distribution<> m_unidist;//integer uniform distribution
    std::uniform_real_distribution<double> m_uniReal;//real uniform distribution

};

unsigned int hashCode(const string str);

class Tester{
    public:
    bool testLinearNoncolidingInsert(VacDB db, vector<Patient> dataList) {
        Patient data1 = Patient("john", 2500, true);
        Patient data2 = Patient("celina", 3500, true);
        Patient data3 = Patient("serina", 2000, true);
        Patient data4 = Patient("mike", 3000, true);

        db.insert(data1);
        dataList.push_back(data1);
        db.insert(data2);
        dataList.push_back(data2);
        db.insert(data3);
        dataList.push_back(data3);
        db.insert(data4);
        dataList.push_back(data4);
        //db.dump();
        bool result = true;
        if (db.m_currentTable[hashCode(data1.getKey()) % db.m_currentCap]->m_serial != 2500) {
            result = false;
        }
        else if (db.m_currentTable[hashCode(data2.getKey()) % db.m_currentCap]->m_serial != 3500) {
            result = false;
        }
        else if (db.m_currentTable[hashCode(data3.getKey()) % db.m_currentCap]->m_serial != 2000) {
            result = false;
        }
        else if (db.m_currentTable[hashCode(data4.getKey()) % db.m_currentCap]->m_serial != 3000) {
            result = false;
        }
        
        
        return result;
    }
};



string namesDB[6] = {"john", "serina", "mike", "celina", "alexander", "jessica"};

// boolPrint(bool cond)
// Helper function to print test results as strings
string boolPrint(bool cond) {
    return (cond) ? "PASSED #" : "FALSE #";
}

int main(){
    
    Random RndID(MINID,MAXID);
    Random RndName(0,5);// selects one from the namesDB array
    Random RndQuantity(0,50);
    Tester test;
    
    // bools that store the results of tests
    bool insertLinearNoncoliding = true;
    bool getPatientError = true;
    bool getPatientNormal = true;
    bool getPatientNormalColliding = true;
    bool removeNormal = true;
    bool removeNormalColliding = true;
    bool removeError = true;
    bool rehashInsertion = true;
    bool rehashRemove = true;
    bool updateSerialNormal = true;

    cout << "TESTING INSERTS\n";
    {
        cout << "Insert non-colliding\n";
        VacDB db(MINPRIME, hashCode, LINEAR);
        vector<Patient> dataList;

        insertLinearNoncoliding = test.testLinearNoncolidingInsert(db, dataList);
        
    }

    cout << "TESTING getPatient\n";
    {
        cout << "getPatient Error Case\n";
        // getPatient Error Case, non-colliding
        // Error case, when patient doesnt exist, it should output empty Patient

        VacDB db(MINPRIME, hashCode, LINEAR);

        Patient data1 = Patient("john", 2500, true);
        Patient data2 = Patient("celina", 3500, true);
        Patient data3 = Patient("serina", 2000, true);
        Patient data4 = Patient("mike", 3000, true);
        Patient empty = Patient();
        
        // insert patients
        db.insert(data1);
        db.insert(data2);
        db.insert(data3);
        db.insert(data4);

        // test if getPatient outputs empty patient
        getPatientError = (empty == db.getPatient("alexander", 5000));
    }

    {
        cout << "getPatient Normal Case, with non-coliding keys\n";
        // getPatient Normal Case, with non-colliding keys

        VacDB db(MINPRIME, hashCode, LINEAR);

        Patient data1 = Patient("john", 2500, true);
        Patient data2 = Patient("celina", 3500, true);
        Patient data3 = Patient("serina", 2000, true);
        Patient data4 = Patient("mike", 3000, true);
        
        // inserts patients
        db.insert(data1);
        db.insert(data2);
        db.insert(data3);
        db.insert(data4);

        // tests if getPatient outputs correct patients
        getPatientNormal = getPatientNormal && (data1 == db.getPatient("john", 2500));
        getPatientNormal = getPatientNormal && (data2 == db.getPatient("celina", 3500));
        getPatientNormal = getPatientNormal && (data3 == db.getPatient("serina", 2000));
        getPatientNormal = getPatientNormal && (data4 == db.getPatient("mike", 3000));
    }

    {
        cout << "getPatient Normal Case, with colliding keys\n";
        // getPatient Normal Case, with colliding keys

        VacDB db(MINPRIME, hashCode, LINEAR);

        Patient data1 = Patient("john", 2501, true);
        Patient data2 = Patient("john", 2502, true);
        Patient data3 = Patient("john", 2503, true);
        Patient data4 = Patient("john", 2504, true);
        Patient data5 = Patient("john", 2505, true);

        // inserts patients
        db.insert(data1);
        db.insert(data2);
        db.insert(data3);
        db.insert(data4);
        db.insert(data5);

        // tests if getPatient outputs the right patients
        getPatientNormalColliding = getPatientNormalColliding && (data1 == db.getPatient("john", 2501));
        getPatientNormalColliding = getPatientNormalColliding && (data2 == db.getPatient("john", 2502));
        getPatientNormalColliding = getPatientNormalColliding && (data3 == db.getPatient("john", 2503));
        getPatientNormalColliding = getPatientNormalColliding && (data4 == db.getPatient("john", 2504));
        getPatientNormalColliding = getPatientNormalColliding && (data5 == db.getPatient("john", 2505));
    }

    cout << "SERIAL UPDATE TESTS\n";
    {
        cout << "updateSerialNumber Normal Case\n";
        // updateSerialNumber normal case

        VacDB db(MINPRIME, hashCode, LINEAR);

        Patient data1 = Patient("john", 2500, true);
        Patient data2 = Patient("celina", 3500, true);
        Patient data3 = Patient("serina", 2000, true);
        Patient data4 = Patient("mike", 3000, true);
        
        // inserts patients
        db.insert(data1);
        db.insert(data2);
        db.insert(data3);
        db.insert(data4);

        // tests if getPatient outputs the right patients
        updateSerialNormal = updateSerialNormal && (db.updateSerialNumber(data1, 2501)); 
    }

    cout << "TESTING REMOVE\n";
    {
        cout << "remove Normal Case, with non-colliding keys\n";
        VacDB db(MINPRIME, hashCode, LINEAR);

        Patient data1 = Patient("john", 2500, true);
        Patient data2 = Patient("celina", 3500, true);
        Patient data3 = Patient("serina", 2000, true);
        Patient data4 = Patient("mike", 3000, true);
        
        // inserts patients
        db.insert(data1);
        db.insert(data2);
        db.insert(data3);
        db.insert(data4);

        // test if remove works 
        removeNormal = removeNormal && (db.remove(data1));
        removeNormal = removeNormal && (db.remove(data2));
        removeNormal = removeNormal && (db.remove(data3));
        removeNormal = removeNormal && (db.remove(data4));
    }

    {
        cout << "remove Normal Case, with colliding keys\n";
        VacDB db(MINPRIME, hashCode, LINEAR);

        Patient data1 = Patient("john", 2500, true);
        Patient data2 = Patient("celina", 3500, true);
        Patient data3 = Patient("serina", 2000, true);
        Patient data4 = Patient("mike", 3000, true);
        Patient data5 = Patient("serina", 2001, true);
        Patient data6 = Patient("mike", 3001, true);
        Patient data7 = Patient("serina", 2002, true);
        Patient data8 = Patient("mike", 3002, true);
        
        // inserts data
        db.insert(data1);
        db.insert(data2);
        db.insert(data3);
        db.insert(data4);
        db.insert(data5);
        db.insert(data6);
        db.insert(data7);
        db.insert(data8);

        // tests remove()
        removeNormalColliding = removeNormalColliding && (db.remove(data1));
        removeNormalColliding = removeNormalColliding && (db.remove(data2));
        removeNormalColliding = removeNormalColliding && (db.remove(data3));
        removeNormalColliding = removeNormalColliding && (db.remove(data4));
        removeNormalColliding = removeNormalColliding && (db.remove(data5));
        removeNormalColliding = removeNormalColliding && (db.remove(data6));
        removeNormalColliding = removeNormalColliding && (db.remove(data7));
    }

    {
        cout << "remove Error Case, with colliding keys\n";
        VacDB db(MINPRIME, hashCode, LINEAR);

        Patient data1 = Patient("john", 2500, true);
        Patient data2 = Patient("celina", 3500, true);
        Patient data3 = Patient("serina", 2000, true);
        Patient data4 = Patient("mike", 3000, true);
        Patient data5 = Patient("serina", 2001, true);
        Patient data6 = Patient("mike", 3001, true);
        Patient data7 = Patient("serina", 2002, true);
        Patient data8 = Patient("mike", 3002, true);
        Patient errorPatient = Patient(); // empty patient to remove

        // inserts patients
        db.insert(data1);
        db.insert(data2);
        db.insert(data3);
        db.insert(data4);
        db.insert(data5);
        db.insert(data6);
        db.insert(data7);
        db.insert(data8);

        // tests if remove will output false on an empty patient
        removeError = removeError && !(db.remove(errorPatient));
    }

    cout << "TESTING REHASH\n";
    {
        cout << "reHash() from insert Tests:\n";
        VacDB db(MINPRIME, hashCode, LINEAR);
        vector<Patient> dataList;

        // loop that adds random Patients to VacDB
        for (int i = 0; i < 70; i++){
            // generating random data
            Patient dataObj = Patient(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
            // saving data for later use
            dataList.push_back(dataObj);
            // inserting data in to the VacDB object
            rehashInsertion = db.insert(dataObj);
        }

        // checks if every patient is inserted
        for (vector<Patient>::iterator it = dataList.begin(); it != dataList.end(); it++){
            rehashInsertion = rehashInsertion && (*it == db.getPatient((*it).getKey(), (*it).getSerial()));
        }
    }

    {
        cout << "reHash() from removal Tests:\n";
        VacDB db(MINPRIME, hashCode, LINEAR);
        vector<Patient> dataList;

        // loop that adds random Patients to VacDB
        for (int i = 0; i < 30; i++){
            // generating random data
            Patient dataObj = Patient(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
            // saving data for later use
            dataList.push_back(dataObj);
            // inserting data in to the VacDB object
            rehashRemove = db.insert(dataObj);
        }

        // checks if every patient is inserted
        for (int i = 0; i < 25; i++) {
            rehashRemove = rehashRemove && (db.remove(dataList.at(i)));
        }
    }


            
    cout << "##################### TEST RESULTS #####################\n"
        << "# Insert Tests:\t\t\t\t\t" << boolPrint(insertLinearNoncoliding) << "\n"
        << "# getPatient() Error Case:\t\t\t" << boolPrint(getPatientError) << "\n"
        << "# getPatient() Normal Case, non-colliding:\t" << boolPrint(getPatientNormal) << "\n"
        << "# getPatient() Error Case, colliding:\t\t" << boolPrint(getPatientNormalColliding) << "\n"
        << "# remove() Normal Case, non-colliding:\t\t" << boolPrint(removeNormal) << "\n"
        << "# remove() Normal Case, colliding:\t\t" << boolPrint(removeNormalColliding) << "\n"
        << "# remove() Error Case, colliding:\t\t" << boolPrint(removeError) << "\n"
        << "# reHash() rehash triggered with insert:\t" << boolPrint(rehashInsertion) << "\n"
        << "# reHash() rehash triggered with remove:\t" << boolPrint(rehashRemove) << "\n"

        << "########################################################\n";
    return 0;
}

unsigned int hashCode(const string str) {
    unsigned int val = 0 ;
    const unsigned int thirtyThree = 33 ;  // magic number from textbook
    for (unsigned int i = 0 ; i < str.length(); i++)
        val = val * thirtyThree + str[i] ;
    
    return val ;
}
