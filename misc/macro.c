#define myfun(fn, t1, a1) void func##fn(t1 ##a1) { printf(a1); }

#define debug(format, ...) fprintf (stderr, format, __VA_ARGS__)

#define RCALL(fn, ...) void call##fn(myarg, __VA_ARGS__) { 
#define RCALL2(fn, mn, ...) return request(this, &Cname::handler##fn(__VA_ARGS__); }

#define RC1(fn) void call##fn(RespHand rDesc,
#define RC2(fn, mn) ) { return request(this, handler##fn(mn, 
#define RC3 ); }

#define RCA2(fn, mn, t1, a1, t2, a2) RC1(fn) t1 a1, t2 a2 RC2(fn, mn) a1, a2 RC3

RCALL(SampleAdd, int a, int b) RCALL2(SampleAdd, "sample.add", a, b)

RC1(SampleAdd) int a, int b RC2("sample.add") a, b RC3

RCA2(SampleAdd, "sample.add", int, a, int, b)

int main()
{
	int abc=10;
	myfun(dummy, int, abc);

	debug("%d, %s", 10, "hello");

}


