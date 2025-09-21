#ifndef EE_VOLVE_H
#define EE_VOLVE_H

#include "ee_array.h"

typedef enum DistrType
{
	UNIFORM = 1,
	NORMAL  = 2,
	CUSTOM  = 3,
} DistrType;

typedef enum SelectType
{
	TOP_K,
} SelectType;

typedef struct DistrUniform
{
	f64 a;
	f64 b;
} DistrUniform;

typedef struct DistrNormal
{
	f64 mean;
	f64 std;
} DistrNormal;

typedef struct Distr
{
	DistrType type;

	union
	{
		DistrUniform uniform;
		DistrNormal normal;
	};

	void* context;
} Distr;

typedef struct PopulationInfo
{
	size_t gens_len;
	size_t pop_len;
	Distr distr;
} PopulationInfo;

typedef struct Ops
{
	f64 (*ee_distr_sample_fn)(Distr* distr, void* user);
} Ops;

typedef struct Agent
{
	Array gens;
	f64 score;
	void* context;
} Agent;

typedef struct Env
{
	Array agents;
	Ops ops;
} Env;

#endif // EE_VOLVE_H
