#ifndef DB_H
#define DB_H

class DB {
public:
    DB(); // Constructor

    DynamicJsonDocument** get(String barcode);
    bool add(DynamicJsonDocument* doc);
    bool set(uint32_t id, String key, String value);
    bool remove(uint32_t id); // Changed the function name to 'remove' due to 'delete' being a reserved keyword
};

#endif // DB_H
