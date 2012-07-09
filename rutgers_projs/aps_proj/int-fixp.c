#include "fixpoint.h"

main()
{
    fixpoint        x,
                    y,
                    z;

    z = x + y;
    x = 1.22;
    y = 3.4;
    z = fp_multiply(x, y);
    x = 4.55;
    y = 322.54;
    z = fp_div(x, y);

    x = 1.32;
    y = 8.44;
    z = fp_sqrt(fp_multiply(x, x) + fp_multiply(y, y));


}
