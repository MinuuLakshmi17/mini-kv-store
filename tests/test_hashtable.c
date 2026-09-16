#include "../src/hashtable.h"

#include <stdio.h>
#include <string.h>

static int test_create_destroy(void)
{
    printf("Test: create and destroy...\n");

    HashTable *table = hashtable_create(8);

    if (table == NULL) {
        fprintf(stderr, "FAIL: could not create table\n");
        return 0;
    }

    if (table->capacity != 8) {
        fprintf(stderr, "FAIL: incorrect initial capacity\n");
        hashtable_destroy(table);
        return 0;
    }

    if (table->size != 0) {
        fprintf(stderr, "FAIL: initial size is not zero\n");
        hashtable_destroy(table);
        return 0;
    }

    for (size_t i = 0; i < table->capacity; i++) {
        if (table->buckets[i] != NULL) {
            fprintf(stderr, "FAIL: bucket %zu is not empty\n", i);
            hashtable_destroy(table);
            return 0;
        }
    }

    hashtable_destroy(table);

    printf("PASS\n");
    return 1;
}

static int test_set_and_get(void)
{
    printf("Test: set and get...\n");

    HashTable *table = hashtable_create(8);

    if (table == NULL) {
        fprintf(stderr, "FAIL: could not create table\n");
        return 0;
    }

    if (hashtable_set(table, "name", "Joseph") != 0) {
        fprintf(stderr, "FAIL: set failed\n");
        hashtable_destroy(table);
        return 0;
    }

    if (table->size != 1) {
        fprintf(stderr, "FAIL: expected size 1, got %zu\n",
                table->size);
        hashtable_destroy(table);
        return 0;
    }

    const char *value = hashtable_get(table, "name");

    if (value == NULL || strcmp(value, "Joseph") != 0) {
        fprintf(stderr, "FAIL: incorrect retrieved value\n");
        hashtable_destroy(table);
        return 0;
    }

    if (hashtable_get(table, "missing") != NULL) {
        fprintf(stderr, "FAIL: missing key returned a value\n");
        hashtable_destroy(table);
        return 0;
    }

    hashtable_destroy(table);

    printf("PASS\n");
    return 1;
}

static int test_update(void)
{
    printf("Test: update existing key...\n");

    HashTable *table = hashtable_create(8);

    if (table == NULL) {
        fprintf(stderr, "FAIL: could not create table\n");
        return 0;
    }

    if (hashtable_set(table, "name", "Joseph") != 0 ||
        hashtable_set(table, "name", "Rodrigo") != 0) {
        fprintf(stderr, "FAIL: set/update failed\n");
        hashtable_destroy(table);
        return 0;
    }

    if (table->size != 1) {
        fprintf(stderr,
                "FAIL: duplicate key increased size to %zu\n",
                table->size);
        hashtable_destroy(table);
        return 0;
    }

    const char *value = hashtable_get(table, "name");

    if (value == NULL || strcmp(value, "Rodrigo") != 0) {
        fprintf(stderr, "FAIL: value was not updated\n");
        hashtable_destroy(table);
        return 0;
    }

    hashtable_destroy(table);

    printf("PASS\n");
    return 1;
}

static int test_exists(void)
{
    printf("Test: exists...\n");

    HashTable *table = hashtable_create(8);

    if (table == NULL) {
        fprintf(stderr, "FAIL: could not create table\n");
        return 0;
    }

    if (hashtable_exists(table, "name") != 0) {
        fprintf(stderr, "FAIL: missing key reported as existing\n");
        hashtable_destroy(table);
        return 0;
    }

    if (hashtable_set(table, "name", "Joseph") != 0) {
        fprintf(stderr, "FAIL: could not insert key\n");
        hashtable_destroy(table);
        return 0;
    }

    if (hashtable_exists(table, "name") != 1) {
        fprintf(stderr, "FAIL: existing key not detected\n");
        hashtable_destroy(table);
        return 0;
    }

    hashtable_destroy(table);

    printf("PASS\n");
    return 1;
}

static int test_delete(void)
{
    printf("Test: delete...\n");

    HashTable *table = hashtable_create(8);

    if (table == NULL) {
        fprintf(stderr, "FAIL: could not create table\n");
        return 0;
    }

    if (hashtable_set(table, "name", "Joseph") != 0 ||
        hashtable_set(table, "language", "C") != 0) {
        fprintf(stderr, "FAIL: could not insert test data\n");
        hashtable_destroy(table);
        return 0;
    }

    if (table->size != 2) {
        fprintf(stderr, "FAIL: expected size 2\n");
        hashtable_destroy(table);
        return 0;
    }

    if (hashtable_delete(table, "name") != 1) {
        fprintf(stderr, "FAIL: existing key was not deleted\n");
        hashtable_destroy(table);
        return 0;
    }

    if (hashtable_get(table, "name") != NULL) {
        fprintf(stderr, "FAIL: deleted key still exists\n");
        hashtable_destroy(table);
        return 0;
    }

    if (table->size != 1) {
        fprintf(stderr, "FAIL: incorrect size after deletion\n");
        hashtable_destroy(table);
        return 0;
    }

    if (hashtable_delete(table, "missing") != 0) {
        fprintf(stderr, "FAIL: missing key reported as deleted\n");
        hashtable_destroy(table);
        return 0;
    }

    if (table->size != 1) {
        fprintf(stderr, "FAIL: deleting missing key changed size\n");
        hashtable_destroy(table);
        return 0;
    }

    hashtable_destroy(table);

    printf("PASS\n");
    return 1;
}

static int test_invalid_input(void)
{
    printf("Test: invalid input...\n");

    HashTable *table = hashtable_create(8);

    if (table == NULL) {
        fprintf(stderr, "FAIL: could not create table\n");
        return 0;
    }

    if (hashtable_create(0) != NULL) {
        fprintf(stderr, "FAIL: zero capacity was accepted\n");
        hashtable_destroy(table);
        return 0;
    }

    if (hashtable_set(NULL, "key", "value") != -1 ||
        hashtable_set(table, NULL, "value") != -1 ||
        hashtable_set(table, "key", NULL) != -1) {
        fprintf(stderr, "FAIL: invalid SET input was accepted\n");
        hashtable_destroy(table);
        return 0;
    }

    if (hashtable_get(NULL, "key") != NULL ||
        hashtable_get(table, NULL) != NULL) {
        fprintf(stderr, "FAIL: invalid GET input was accepted\n");
        hashtable_destroy(table);
        return 0;
    }

    if (hashtable_exists(NULL, "key") != 0 ||
        hashtable_exists(table, NULL) != 0) {
        fprintf(stderr, "FAIL: invalid EXISTS input was accepted\n");
        hashtable_destroy(table);
        return 0;
    }

    if (hashtable_delete(NULL, "key") != 0 ||
        hashtable_delete(table, NULL) != 0) {
        fprintf(stderr, "FAIL: invalid DELETE input was accepted\n");
        hashtable_destroy(table);
        return 0;
    }

    hashtable_destroy(table);

    printf("PASS\n");
    return 1;
}

static int test_resize(void)
{
    printf("Test: resize and rehash...\n");

    HashTable *table = hashtable_create(4);

    if (table == NULL) {
        fprintf(stderr, "FAIL: could not create table\n");
        return 0;
    }

    /*
     * At a 75% load factor, inserting the fourth key into a
     * four-bucket table forces a resize.
     */
    if (hashtable_set(table, "key1", "value1") != 0 ||
        hashtable_set(table, "key2", "value2") != 0 ||
        hashtable_set(table, "key3", "value3") != 0 ||
        hashtable_set(table, "key4", "value4") != 0) {
        fprintf(stderr, "FAIL: insertion failed during resize test\n");
        hashtable_destroy(table);
        return 0;
    }

    if (table->capacity != 8) {
        fprintf(stderr,
                "FAIL: expected capacity 8 after resize, got %zu\n",
                table->capacity);
        hashtable_destroy(table);
        return 0;
    }

    if (table->size != 4) {
        fprintf(stderr,
                "FAIL: expected size 4, got %zu\n",
                table->size);
        hashtable_destroy(table);
        return 0;
    }

    const char *expected[] = {
        "value1",
        "value2",
        "value3",
        "value4"
    };

    for (int i = 0; i < 4; i++) {
        char key[16];

        snprintf(key, sizeof(key), "key%d", i + 1);

        const char *value = hashtable_get(table, key);

        if (value == NULL || strcmp(value, expected[i]) != 0) {
            fprintf(stderr,
                    "FAIL: key %s was corrupted during resize\n",
                    key);
            hashtable_destroy(table);
            return 0;
        }
    }

    hashtable_destroy(table);

    printf("PASS\n");
    return 1;
}

static int test_collision_chaining(void)
{
    printf("Test: collision chaining...\n");

    /*
     * A very small table makes collisions much more likely.
     */
    HashTable *table = hashtable_create(2);

    if (table == NULL) {
        fprintf(stderr, "FAIL: could not create table\n");
        return 0;
    }

    const char *keys[] = {
        "alpha",
        "beta",
        "gamma",
        "delta",
        "epsilon"
    };

    const char *values[] = {
        "one",
        "two",
        "three",
        "four",
        "five"
    };

    for (size_t i = 0; i < 5; i++) {
        if (hashtable_set(table, keys[i], values[i]) != 0) {
            fprintf(stderr,
                    "FAIL: could not insert collision-test key\n");
            hashtable_destroy(table);
            return 0;
        }
    }

    /*
     * Verify that every key remains accessible even when
     * multiple keys share buckets.
     */
    for (size_t i = 0; i < 5; i++) {
        const char *value = hashtable_get(table, keys[i]);

        if (value == NULL || strcmp(value, values[i]) != 0) {
            fprintf(stderr,
                    "FAIL: collision chain corrupted key %s\n",
                    keys[i]);
            hashtable_destroy(table);
            return 0;
        }
    }

    /*
     * Delete one key and verify the other entries remain intact.
     */
    if (hashtable_delete(table, "gamma") != 1) {
        fprintf(stderr, "FAIL: collision-chain deletion failed\n");
        hashtable_destroy(table);
        return 0;
    }

    if (hashtable_get(table, "gamma") != NULL) {
        fprintf(stderr, "FAIL: deleted collision key still exists\n");
        hashtable_destroy(table);
        return 0;
    }

    if (hashtable_get(table, "alpha") == NULL ||
        hashtable_get(table, "beta") == NULL ||
        hashtable_get(table, "delta") == NULL ||
        hashtable_get(table, "epsilon") == NULL) {
        fprintf(stderr,
                "FAIL: deleting one collision entry damaged others\n");
        hashtable_destroy(table);
        return 0;
    }

    hashtable_destroy(table);

    printf("PASS\n");
    return 1;
}

int main(void)
{
    int passed = 0;
    const int total = 8;

    passed += test_create_destroy();
    passed += test_set_and_get();
    passed += test_update();
    passed += test_exists();
    passed += test_delete();
    passed += test_invalid_input();
    passed += test_resize();
    passed += test_collision_chaining();

    printf("\nResult: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}