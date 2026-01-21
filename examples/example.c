#define __EXAMPLE_C__

/*
 * An example code for az library
 */

#include <stdio.h>

#include <arikkei/arikkei-strlib.h>

#include <az/base.h>
#include <az/extend.h>
#include <az/packed-value.h>

/*
 * Given full type data - implementation and instance, print:
 * Class (type) name
 * Instanmce size
 * Value size
 * String representation of actual data
 */

 static void
print_type_info(const AZImplementation *impl, void *inst)
{
    /* Get a class pointer (the same as impl for standalone types, separate for interfaces )*/
    AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
    /* Print some class information */
    fprintf(stdout, "Instance of %s\n", klass->name);
    fprintf(stdout, "  Instance size %u\n", klass->instance_size);
    fprintf(stdout, "  Value size %u\n", az_class_value_size(klass));
    /* Get the string representation of value (uses to_string virtual method internally) */
    uint8_t buf[256];
    az_instance_to_string(impl, inst, buf, 256);
    fprintf(stdout, "  Value: %s\n", buf);
}

/*
 * Matrix type
 */
typedef struct {
    float c[9];
} Matrix;

/*
 * to_string method for Matrix class
 *
 * Outputs "(a11, a12 ... a33)
 */
unsigned int
matrix_to_string (const AZImplementation* impl, void *inst, unsigned char *d, unsigned int d_len)
{
    const Matrix *mat = (const Matrix *) inst;
	unsigned int pos = 0, i;
	if (pos < d_len) d[pos++] = '(';
	for (i = 0; i < 8; i++) {
		pos += arikkei_dtoa_exp (d + pos, (d_len > pos) ? d_len - pos : 0, mat->c[i], 6, -5, 5);
		if (pos < d_len) d[pos++] = ',';
	}
	pos += arikkei_dtoa_exp (d + pos, (d_len > pos) ? d_len - pos : 0, mat->c[8], 6, -5, 5);
	if (pos < d_len) d[pos++] = ')';
	if (pos < d_len) d[pos] = 0;
	return pos;
}

/*
 * Matrix class constructor
 */
static void
matrix_class_init(AZClass *klass)
{
    /*
     * Set the virtual method 'to_string' to proper value.
     * In given case we could as do that later, after the type creation, but some other actions -
     * like interface declarations - have to be done in class constructor
     */
    klass->to_string = matrix_to_string;
}

/*
 * Matrix instance constructor
 */
static void
matrix_init(const AZClass *klass, Matrix *mat)
{
    *mat = (Matrix) {1, 0, 0, 0, 1, 0, 0, 0, 1};
}

int
main(int argc, const char *argv[])
{
    /*
     * Initialize az
     *
     * As we start immediately using the primitive types, it is not done automatically
     */
    az_init();

    /*
     * Case 1 - use value directly
     */
    fprintf (stdout, "Primitive types\n");

     /* An C primitive value */
    uint32_t u32_val = 123456;
    /* To use the type information we need uint32 implementation */
    const AZImplementation *u32_impl = AZ_IMPL_FROM_TYPE(AZ_TYPE_UINT32);
    /* Print information about the type (refer above) */
    print_type_info(u32_impl, &u32_val);

    double d_val = 1.02030405;
    print_type_info(AZ_IMPL_FROM_TYPE(AZ_TYPE_DOUBLE), &d_val);

    AZComplexDouble cd_val = {1.2, 3.4};
    print_type_info(AZ_IMPL_FROM_TYPE(AZ_TYPE_COMPLEX_DOUBLE), &cd_val);

    /*
     * Case 2 - Use a packed value
     *
     * AZPackedValue is meant for easy storage of type information up to 16 bytes in sisze.
     * It contains both the implementation pointer and value of a type.
     */
    fprintf (stdout, "\nPrimitive types in packed values\n");

    AZPackedValue u32_pval = {.impl = AZ_IMPL_FROM_TYPE(AZ_TYPE_UINT32), .v.uint32_v = 654321};
    print_type_info(u32_pval.impl, az_packed_value_get_inst(&u32_pval));

    /* Set a packed value from type/instance */
    /* NB! A new packed value has to be initialized to zero */
    AZPackedValue i32_pval = {0};
    int32_t i32 = -123456;
    az_packed_value_set_from_type(&i32_pval, AZ_TYPE_INT32, &i32);
    print_type_info(i32_pval.impl, az_packed_value_get_inst(&i32_pval));

    /*
     * Case 3 - create a new value type
     *
     * We create a simple matrix type with constructor to unit matrix and to_string method
     */
    fprintf (stdout, "\nA composite value type\n");

    /* A type value will be stored here after creation */
    unsigned int matrix_type;
    /* Register the new type, assign typecode and create the class */
    AZClass *matrix_class = az_register_type(
        /* Pointer to resulting type value */
        &matrix_type,
        /* Type name */
        (const uint8_t *) "Matrix",
        /* Parent type - we inherit directly from struct */
        AZ_TYPE_STRUCT,
        /* Class and instance sizes */
        sizeof(AZClass), sizeof(Matrix),
        /* Type flags */
        0,
        /* Class constructor */
        (void (*) (AZClass *)) matrix_class_init,
        /* Instance constructor */
        (void (*) (const AZImplementation *, void *)) matrix_init,
        /* Instance destructor */
        NULL);
    /* Create a matrix type in stack */
    Matrix mat;
    /* Call constructor */
    az_instance_init_by_type(&mat, matrix_type);
    /* Print type information */
    print_type_info(&matrix_class->impl, &mat);

    /* Use AZPackedValue64 to store matrix */
    /* NB! 1. As Matrix is a value class, the actual value is copied */
    /* NB! 2. As Matrix has instance size 36 bytes, it cannot be stored inside AZPacked value */
    /* So we should use AZPackedValue64 for it */
    AZPackedValue64 m_pval = {0};
    az_packed_value_set_from_type(&m_pval.packed_val, matrix_type, &mat);
    print_type_info(m_pval.impl, az_packed_value_get_inst(&m_pval.packed_val));

    /*
     * Case 5 - create a new block type
     */
    fprintf (stdout, "\nA composite block type\n");
    unsigned int b_matrix_type;
    AZClass *b_matrix_class = az_register_type(&b_matrix_type, (const uint8_t *) "Matrix block", AZ_TYPE_BLOCK, sizeof(AZClass), sizeof(Matrix),
        0,
        (void (*) (AZClass *)) matrix_class_init,
        (void (*) (const AZImplementation *, void *)) matrix_init,
        NULL);
    Matrix *b_mat;
    /* Call constructor */
    /* As Block matrix is a block type, we should use new */
    b_mat = (Matrix *) az_instance_new(b_matrix_type);
    print_type_info(&b_matrix_class->impl, b_mat);

    /* Use AZPackedValue to store matrix */
    /* Block type value size is 8 so we can use simple PackedValue */
    AZPackedValue bm_pval = {0};
    az_packed_value_set_from_type(&bm_pval, b_matrix_type, b_mat);
    print_type_info(bm_pval.impl, az_packed_value_get_inst(&bm_pval));

    /*
     * Now we can check what happens if we change instance of value and block types
     */
    fprintf (stdout, "\nValue/Block type behavior\n");

    /* Modify the value type */
    mat.c[0] = -1;
    fprintf (stdout, "The modified value matrix\n");
    print_type_info(&matrix_class->impl, &mat);
    fprintf (stdout, "A value copy in packed value\n");
    print_type_info(m_pval.impl, az_packed_value_get_inst(&m_pval.packed_val));

    /* Modify the block type */
    b_mat->c[4] = -1;
    fprintf (stdout, "The modified block matrix\n");
    print_type_info(&b_matrix_class->impl, b_mat);
    fprintf (stdout, "A value copy in packed value\n");
    print_type_info(bm_pval.impl, az_packed_value_get_inst(&bm_pval));

    /* No-op here but some instances may have destructors */
    az_instance_finalize_by_type(&mat, matrix_type);
    /* Delete block matrix */
    az_instance_delete(b_matrix_type, b_mat);

    return 0;
}