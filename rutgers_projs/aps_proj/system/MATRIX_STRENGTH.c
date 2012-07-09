#ifdef FULLPC
#include <stdlib.h>
#include <string.h>
#endif

#include "tos.h"
#include "MATRIX_STRENGTH.h"

#ifdef FULLPC
TOS_FRAME_BEGIN(MATRIX_STRENGTH_frame)
{
    int             matrix_size;
    int           **strength_matrix;
}
TOS_FRAME_END(MATRIX_STRENGTH_frame);


void
read_matrix()
{
    FILE           *fp = NULL;
    char            line[64],
                   *chr = NULL,
                   *chr1 = NULL;
    int             i = 0,
                    j = 0,
                    d,
                    d1,
                    d2;

    // Load strength matrix
    if ((fp = fopen("RF_simulator/strength.txt", "r")) == NULL) {
	printf("There is no file %s under directory %s\n", "strength.txt",
	       "RF_simulator");

	return;
    }

    if (!(fgets(line, 64, fp))) {
	printf("File %s is empty\n", "RF_simulator/strength.txt");

	return;
    }

    VAR(matrix_size) = atoi(line);

    if (VAR(matrix_size) <= 1) {
	printf("Matrix size in file %s should be >= 2\n",
	       "RF_simulator/strength.txt");

	return;
    }

    VAR(strength_matrix) =
	(int **) malloc(sizeof(int *) * VAR(matrix_size));

    for (i = 0; i < VAR(matrix_size); i++) {
	VAR(strength_matrix)[i] =
	    (int *) malloc(sizeof(int) * VAR(matrix_size));
    }

    for (i = 0; i < VAR(matrix_size); i++)
	for (j = 0; j < VAR(matrix_size); j++)
	    VAR(strength_matrix)[i][j] = 0;

    while (fgets(line, 64, fp)) {
	if ((chr = strchr(line, '\t')) == NULL) {
	    printf("Line %s has wrong format\n", line);

	    return;
	}

	chr1 = chr + 1;
	*chr = '\0';

	if ((chr1 = strchr(chr1, '\t')) == NULL) {
	    printf("Line %s has wrong format\n", line);

	    return;
	}

	*chr1 = '\0';

	d = atoi(line);
	d1 = atoi(chr + 1);
	d2 = atoi(chr1 + 1);

	if (d < 0) {
	    printf("Invalid node: %d\n", d);

	    return;
	} else if (d1 < 0) {
	    printf("Invalid node: %d\n", d1);

	    return;
	}

	VAR(strength_matrix)[d][d1] = d2;
    }

    fclose(fp);

    printf("Signal matrix loaded\n");
}
#endif


char            TOS_COMMAND(MATRIX_STRENGTH_INIT) () {
#ifdef FULLPC
    read_matrix();
#endif

    return 1;
}

char            TOS_COMMAND(MATRIX_STRENGTH_GET_STRENGTH) (char src) {
#ifdef FULLPC
    int             data;

    data = VAR(strength_matrix)[(int) src][(int) TOS_LOCAL_ADDRESS];

    TOS_SIGNAL_EVENT(MATRIX_STRENGTH_DATA_READY) (data);
#endif

    return 1;
}
