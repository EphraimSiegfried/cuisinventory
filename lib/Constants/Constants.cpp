#include <Constants.h>

const char BARCODE_ENDPOINT[] = "world.openfoodfacts.org";
const char BARCODE_PATH[] = "/api/v3/product/";
const char BARCODE_FIELDS[] =
    "empty,product_name,generic_name,product_quantity,brands,image_url,"
    "categories,empty";
const char USER_AGENT[] = "Cuisinventory/1.0 alexander.lutsch@stud.unibas.ch";

// Pythonanywhere server
const char PYTHONANYWHERE_ENDPOINT[] = "kev1n27.pythonanywhere.com";
const char PYTHONANYWHERE_PATH[] = "/cuisinventory";
const char INTERNAL_FOLDER[] = "/.intern";
const char STATE_FOLDER[] = "/.state";
const char DATA_FOLDER[] = "/.data";
const char SETTINGSFILE[] = "settings.json";
const char ID_BAR_MAPPINGFILE[] = "idbar";
const char BAR_ID_MAPPINGFILE[] = "barid";
const char STATEFILE[] = "state";