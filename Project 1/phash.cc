/**
 * Code is based on
 *http://www.algolist.net/Data_structures/Hash_table/Chaining
 *
 **/


class LinkedHashEntry {
private:
      int key;
      int value;
      LinkedHashEntry *next;
public:
      LinkedHashEntry(int key, int value); 
      int getKey(); 
      int getValue();
      void setValue(int value);
 
      LinkedHashEntry *getNext(); 
      void setNext(LinkedHashEntry *next); 
};


class HashMap {
private:
      LinkedHashEntry **table;
public:
      HashMap(); 
      int get(int key); 
      void put(int key, int value); 
      void remove(int key); 
      ~HashMap(); 
};


