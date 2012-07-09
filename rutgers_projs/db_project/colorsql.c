/* This transforms a 1D array with colored points to a 2D array.
 * The transformation basically converts the instance of the
 * generalized counting/reporting problem to a instance of the
 * standard counting/reporting problem.
 * See Gupta, Janardhan and Smid paper in Algorithms journal 19
 */

/* This version uses the MYSQL database as the backend */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <mysql/mysql.h>
#include <sys/time.h>

#define MAX_PTS    1000000
#define TOTAL_PTS  1000000
#define MIN_P (5)
#define MAX_P (1000)
#define GROUND (0)
//#define TABLE_INIT
#define BIGTEXT2 "\"Indexes are used to find rows with a specific value of one columnfast. Without an index MySQL has to start with the first recordand then read through the whole table until it finds the relevantrows. The bigger the table, the more this costs.\""
#define BIGTEXT "'Hello world'"

typedef long long int int64;
typedef unsigned long long int uint64;

enum colors {
  Black, Blue, White, Red, Yellow, Green, Gray, Pink, Purple,
  a1, a2, a3, a5, a6, a7, a8, a9, a10,
  b1, b2, b3, b5, b6, b7, b8, b9, b10,
  c1, c2, c3, c5, c6, c7, c8, c9, c10,
  d1, d2, d3, d5, d6, d7, d8, d9, d10,
  e1, e2, e3, e5, e6, e7, e8, e9, e10,
  f1, f2, f3, f5, f6, f7, f8, f9, f10,
  g1, g2, g3, g5, g6, g7, g8, g9, g10,
  Last};

/* stores 1D data - color and the point p */
struct gen {
  int64 x[MAX_PTS];
  enum colors color[MAX_PTS];
  int total;
} gen_pts;

/* stores 2D data - (p, pred(p)) and the color */
struct std {
  int64 x[MAX_PTS],
    y[MAX_PTS];
  enum colors color[MAX_PTS];
  unsigned int total;
} std_pts;

/* stores the x^5 + y^5 */
struct penta {
  int64 x5[MAX_PTS];
  int64 x[MAX_PTS];
  enum colors color[MAX_PTS];
  unsigned int total;
} penta_pts;

/* Init the db and connect */
int dbinit(MYSQL *dbase, const char *host, const char *user,
	   const char *passwd, const char *dbname)
{
  /*
   * example that does simple data base lookup. 
   */

  int             err = 0;

  /*
   * connect to the database 
   */

  if (mysql_init(dbase) == NULL)
    err = 1;
  else {
    if (mysql_real_connect(dbase, host, user, passwd,
			   dbname, 0, NULL, 0) == NULL)
      err = 1;
  }
  if (err) {
    return -1;
  }

  return 1;
}

/* sends the query to the db, takes printfstyle args */
inline int dbquery(MYSQL *dbase, const char *format, ...)
{
  char sqlbuff[4096];
  va_list ap;

  /*
   * now construct SQL query in 'sqlbuff' 
   */
  
  va_start (ap, format);
  vsprintf(sqlbuff, format, ap);
  va_end(ap);
  //printf("sending %s\n", sqlbuff);
  
  if (mysql_real_query(dbase, sqlbuff, strlen(sqlbuff))) {
    printf("%s\n", mysql_error(dbase));
    return -1;
  }
  return 1;
}

/* creates tables with proper schema */
int table_init(MYSQL *dbase)
{
  int r;

  /* drop tables gen, std and penta */
  printf("Table init\n");
  dbquery(dbase, "drop table gen");
  dbquery(dbase, "drop table penta");
  dbquery(dbase, "drop table std");

  /* create with proper schema */
  r = dbquery(dbase, "create table gen (color int, x bigint, "
	      "t1 text, t2 text, t3 text, t4 text, t5 text)");
  if (r < 0) return r;

  r = dbquery(dbase, "create table std (color int, x bigint, y bigint, "
	      "t1 text, t2 text, t3 text, t4 text, t5 text)");
  if (r < 0) return r;

  r = dbquery(dbase, "create table penta (color int, x5 bigint, x bigint, "
	      "t1 text, t2 text, t3 text, t4 text, t5 text)");
  return r;

#ifdef FALSE
  /* create proper indices */
  r = dbquery(dbase, "create index genidx on gen (x)");
  if (r < 0) return r;

  r = dbquery(dbase, "create index genidx on std (x, y)");
  if (r < 0) return r;

  r = dbquery(dbase, "create index genidx on penta (x, x5)");
  if (r < 0) return r;
#endif
}

/* prints results in a tabular form */
void printres(MYSQL_RES *result) {
  char           *place;
  int             nrows;
  unsigned int      i,j;
  unsigned int num_fields;
  unsigned long *lengths;

  nrows = mysql_num_rows(result);
  if (nrows == 0) {
    printf("No entries for %s\n", place);
  } else {
    MYSQL_ROW       row;
    for (i = 0; i < nrows; i++) {
      row = mysql_fetch_row(result);
      num_fields = mysql_num_fields(result);
      lengths = mysql_fetch_lengths(result);

      for (j = 0; j < num_fields; j++)
	printf("\t%s", row[j]);
      printf("\n");
    }
    printf("\n");
  }
}

/* adds a random set of points to the gen_pts struct
 * returns the total */
inline void add_gen_pts()
{

  int i, r, color;
  int64 x;

  /* init */
  gen_pts.total = 0;
  srand(TOTAL_PTS);

  for (i=0; i< TOTAL_PTS; i++) {
    r = rand();

    if (gen_pts.total > MAX_PTS) {
      printf("Error, table is full\n");
      exit(1);
    }

    /* the point should fall b/w MIN_P and MAX_P */
    x = (double) MAX_P * r/RAND_MAX + MIN_P;
    color = Last * (double)r/RAND_MAX;

    gen_pts.x[gen_pts.total] = x;
    gen_pts.color[gen_pts.total] = color;
    gen_pts.total++;
    
  }
}

/* returns x^5 */
inline int64 pow5(int64 a)
{
  int64 b = a*a;
  b *= b;
  b *= a;

  return b;
}

/* compares two long longs and retuns -1, 0 or +1 */
inline int compare(const void *a, const void *b) {
  if (*(int64 *)a < *(int64 *)b)
    return -1;
  else if (*(int64 *)a > *(int64 *)b)
    return +1;

  return 0;
}

/* given a set of points in the general form, converts it
 * to the standard form */
/* FIXME: this is not at all optimized */
void gen2std(const struct gen *g, struct std *s)
{
  int total = g->total;
  int i, j, k;
  int color;
  int64 x[MAX_PTS];

  /* for each color, sort the points in gen */
  s->total = 0;
  for (color=0; color < Last; color++) {
    j = 0;
    for (i=0; i<total; i++) {
      if (g->color[i] == color)
	x[j++] = g->x[i];
    }
    /* sort this */
    //printf("Found %d for color %d\n", j, color);
    qsort((void *)x, j, sizeof(*x), compare);

    /* now put this into the std struct */
    // first ele for this color
    if (j) {
      s->color[s->total] = color;
      s->x[s->total] = x[0];
      s->y[s->total] = GROUND;
      s->total++;
    }
    for (k=1; k < j; k++) {
      s->color[s->total] = color;
      s->x[s->total] = x[k];
      s->y[s->total] = x[k-1];
      s->total++;
    }
  }
}

/* Converts the std_pts to x^5 + y^5 */
void std2penta(const struct std *s, struct penta *p)
{
  int i;

  p->total = 0;
  for (i=0; i < s->total; i++) {
    p->x5[i] = pow5(s->x[i]) + pow5(s->y[i]);
    p->x[i] = s->x[i];
    p->color[i] = s->color[i];
    p->total++;
  }
}

/* fills the points from the structure into the gen table
 * returns status of the query execution */
int fill_gen_table(MYSQL *dbase, struct gen *g)
{
  int i, r;

  for (i=0; i<g->total; i++) {
    r = dbquery(dbase, "INSERT INTO gen (color, x, t1, t2, t3, t4, t5) "
		"VALUES (%d, %lld, %s, %s, %s, %s, %s)",
		g->color[i], g->x[i],
		BIGTEXT, BIGTEXT, BIGTEXT, BIGTEXT, BIGTEXT);
    if (r < 0) {
      printf("SQL error\n");
      return r;
    }
  }
  return r;
}

/* fills the penta table and returns query status */
int fill_penta_table(MYSQL *dbase, struct gen *g)
{
  int i, r;

  /* first fill the std struct, then the penta struct */
  gen2std(g, &std_pts);
  std2penta(&std_pts, &penta_pts);

  /* now put the penta struct to penta table */
  for (i=0; i<penta_pts.total; i++) {
    r = dbquery(dbase, "INSERT INTO penta (color, x5, x, t, t2, t3, t4, t5) " 
		"VALUES (%d, %lld, %lld, %s, %s, %s, %s, %s)",
		penta_pts.color[i], penta_pts.x5[i],
		penta_pts.x[i],
		BIGTEXT, BIGTEXT, BIGTEXT, BIGTEXT, BIGTEXT);
    if (r < 0) {
      printf("SQL error\n");
      return r;
    }
  }
  return r; 
}

/* fills the std table and returns query status */
int fill_std_table(MYSQL *dbase, struct gen *g)
{
  int i, r;

  /* fill the std struct */
  gen2std(g, &std_pts);

  /* now put the std struct to std table */
  for (i=0; i<std_pts.total; i++) {
    r = dbquery(dbase, "INSERT INTO std (color, x, y, t1, t2, t3, t4, t5) " 
		"VALUES (%d, %lld, %lld, %s, %s, %s, %s, %s)",
		std_pts.color[i], std_pts.x[i],
		std_pts.y[i],
		BIGTEXT, BIGTEXT, BIGTEXT, BIGTEXT, BIGTEXT);
    if (r < 0) {
      printf("SQL error\n");
      return r;
    }
  }
  return r; 
}

/* inserts points to the data base and prints time */
int time_fills(MYSQL *dbase)
{
  struct timeval tv1, tv2;
  double t;
  int r;

  /* add the points to gen_pts */
  gen_pts.total = std_pts.total = penta_pts.total = 0;
  add_gen_pts();

  /* Fill these points into the gen table of the DB */
  gettimeofday(&tv1, NULL);
  r = fill_gen_table(dbase, &gen_pts);
  if (r<0) {
    printf("fill_gen_table failed\n");
    return -1;
  }
  gettimeofday(&tv2, NULL);
  t = (tv2.tv_sec - tv1.tv_sec) +
    (double) (tv2.tv_usec - tv1.tv_usec) / 1000000;
  printf("Time taken to fill gen table: %lf sec\n", t);

  /* fill these points into the std table */
  gettimeofday(&tv1, NULL);
  r = fill_std_table(dbase, &gen_pts);
  if (r<0) {
    printf("fill_std_table failed\n");
    return -1;
  }
  gettimeofday(&tv2, NULL);
  t = (tv2.tv_sec - tv1.tv_sec) + 
    (double) (tv2.tv_usec - tv1.tv_usec) / 1000000;
  printf("Time taken to fill std table: %lf sec\n", t);

#ifdef FALSE
  /* fill these points into the penta table */
  gettimeofday(&tv1, NULL);
  r = fill_penta_table(dbase, &gen_pts);
  if (r<0) {
    printf("fill_penta_table failed\n");
    return -1;
  }
  gettimeofday(&tv2, NULL);
  t = (tv2.tv_sec - tv1.tv_sec) + 
    (double) (tv2.tv_usec - tv1.tv_usec) / 1000000;
  printf("Time taken to fill penta table: %lf sec\n", t);
#endif

  return 1;

}

/* times the time to execute the queries on the two tables */
int time_query(MYSQL *dbase, int64 start, int64 end)
{
  struct timeval tv1, tv2;
  double t;
  int r;
  int64 starty, start5, endy, end5;
  MYSQL_RES *result;
  
  /* the query for the gen table must use group by
   * on the other hand all queries on penta must give
   * results that are already free of duplicates.
   * So in order to emphasize the duplicate removal,
   * the queries will span the entire range */
  printf("Asking for all colors for x between %lld and %lld\n",
	 start, end);

  /*** Do for the gen table ***/
  gettimeofday(&tv1, NULL);
  r = dbquery(dbase, "SELECT color FROM gen WHERE " 
	      "x>= %lld AND x< %lld GROUP BY color",
	      start, end);
  if (r<0) {
    printf("Query on gen table failed\n");
    return -1;
  }
  result = mysql_store_result(dbase);
  //printres(result);
  mysql_free_result(result);
  gettimeofday(&tv2, NULL);
  t = (tv2.tv_sec - tv1.tv_sec) +
    (double) (tv2.tv_usec - tv1.tv_usec) / 1000000;
  printf("Time taken to QUERY gen table: %lf sec\n", t);

  /*** Do for the std table ***/
  gettimeofday(&tv1, NULL);
  /* (l,r) -> [l,r] x [-oo, l]
   * in our case, -oo is really GROUND */
  starty = GROUND;
  endy = start;
  r = dbquery(dbase, "SELECT color FROM std WHERE " 
	      "y>=%lld AND y<%lld AND "
	      "x>=%lld AND x<%lld",
	      starty, endy,
	      start, end);
  if (r<0) {
    printf("Query on std table failed\n");
    return -1;
  }
  result = mysql_store_result(dbase);
  //printres(result);
  mysql_free_result(result);
  gettimeofday(&tv2, NULL);
  t = (tv2.tv_sec - tv1.tv_sec) + 
    (double) (tv2.tv_usec - tv1.tv_usec) / 1000000;
  printf("Time taken to QUERY std table: %lf sec\n", t);

  /*** Do for the penta table ***/
  gettimeofday(&tv1, NULL);
  /* (l,r) -> [l,r] x [-oo, l] -> (l^5+ -oo^5, l^5 + r^5)
   * in our case, -oo is really GROUND */
  start5 = pow5(start) + pow5(starty);
  end5 = pow5(end) + pow5(endy);
  r = dbquery(dbase, "SELECT color FROM penta WHERE x5>=%lld AND x5<%lld",
	      start5, end5);
  if (r<0) {
    printf("Query on penta table failed\n");
    return -1;
  }
  result = mysql_store_result(dbase);
  //printres(result);
  mysql_free_result(result);
  gettimeofday(&tv2, NULL);
  t = (tv2.tv_sec - tv1.tv_sec) + 
    (double) (tv2.tv_usec - tv1.tv_usec) / 1000000;
  printf("Time taken to QUERY penta table: %lf sec\n", t);

  return 1;
}

/*
 * First add a set of points to the gen_pts struct.
 *
 * Then these points are added into the gen table in the
 * database. The operation is timed.
 * We then convert the gen_pts to penta_pts and add these
 * points to the penta table in the DB. This is also timed.
 *
 * The second part is to query the two tables and time how
 * long it takes to complete each query
 */
int main(int argc, char **argv)
{
  MYSQL dbase;
  int r, i;

  /* connect to DB */
  if (dbinit(&dbase, "localhost", "ashwink",
	     "", "2d") < 0) {
    printf("Error connecting to database\n");
    return -1;
  }

#ifdef TABLE_INIT
  /* table init */
  r = table_init(&dbase);
  if (r < 0) {
    printf("table_init failed\n");
    return -1;
  }

  /* time the adding of points */
  if (time_fills(&dbase) < 0) {
    return -1;
  }
#endif

  /* time the queries */
  if (time_query(&dbase, MIN_P, MAX_P) < 0) {
    return -1;
  }

  return 0;  
}
