#pragma once

const String MAPPINGFILE = "/.intern/mapping";
const String STATEFILE = "/.intern/state";

const String BARCODE_ENDPOINT = "world.openfoodfacts.org";
const String BARCODE_PATH = "/api/v3/product/";
const String BARCODE_FIELDS =
    "product_name,generic_name,allergens,conservation_conditions,nutriscore_"
    "grade,ingredients_text,customer_service";
const String USER_AGENT = "Cuisinventory/1.0 alexander.lutsch@stud.unibas.ch";
