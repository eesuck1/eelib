#include "stdio.h"
#include "limits.h"

#include "ee_array.h"
#include "ee_string.h"
#include "ee_arena.h"
//#include "ee_volve.h"

#define EEV_TARGET_CSTR      ("HELLOEEVOLVE")
#define EEV_GENOME_LEN       (sizeof(EEV_TARGET_CSTR) - 1)
#define EEV_MAX_AGENTS       (100)
#define EEV_MAX_ITERS        (1000)
#define EEV_BEST_AGENTS      (10)
#define EEV_MUTATION_PROB    (20)

typedef struct Agent
{
    Str genome;
    s32 score;
} Agent;

EE_INLINE Agent eev_agent_copy(const Agent* src, Allocator* allocator)
{
    Agent out = { 0 };

    out.genome = ee_str_copy(&src->genome, allocator);
    out.score = src->score;

    return out;
}

EE_INLINE void eev_init_genome(Agent* agent)
{
    for (size_t i = 0; i < agent->genome.cap; ++i)
    {
        u8 symbol = 'A' + rand() % 26;

        ee_str_push(&agent->genome, symbol);
    }
}

EE_INLINE s32 eev_score_cmp(const void* a_ptr, const void* b_ptr)
{
    const Agent* a = a_ptr;
    const Agent* b = b_ptr;

    return a->score - b->score;
}

EE_INLINE void eev_cross_and_mutate(const Agent* parent_0, const Agent* parent_1, Agent* child)
{
    for (size_t i = 0; i < EEV_GENOME_LEN; ++i)
    {
        if ((rand() % 100) < EEV_MUTATION_PROB)
        {
            ee_str_set(&child->genome, i, 'A' + rand() % 26);
        }
        else
        {
            if (i & 1)
                ee_str_set(&child->genome, i, ee_str_get(&parent_0->genome, i));
            else
                ee_str_set(&child->genome, i, ee_str_get(&parent_1->genome, i));
        }
    }

    child->score = INT_MAX;
}

void run_hello_world()
{
    Arena gen_arena = ee_arena_new(EEV_MAX_AGENTS * EEV_GENOME_LEN * 2, EE_NO_REWIND, NULL);
    Arena best_gen_arena = ee_arena_new(EEV_BEST_AGENTS * EEV_GENOME_LEN * 2, EE_NO_REWIND, NULL);
    
    Allocator gen_alloc = ee_arena_allocator(&gen_arena);
    Allocator best_gen_alloc = ee_arena_allocator(&best_gen_arena);
    
    Array agents = ee_array_new(EEV_MAX_AGENTS, sizeof(Agent), NULL);
    Agent best_agents[EEV_BEST_AGENTS] = { 0 };
    
    Str target_str = ee_str_from_cstr(EEV_TARGET_CSTR, NULL);

    size_t last_iter = EEV_MAX_ITERS;
    s32 best_score = INT_MAX;

    for (size_t i = 0; i < EEV_MAX_AGENTS; ++i)
    {
        Agent agent = { ee_str_new(EEV_GENOME_LEN, &gen_alloc), INT_MAX };

        eev_init_genome(&agent);
        ee_array_push(&agents, EE_ARRAY_DT(agent));
    }

    for (size_t i = 0; i < EEV_MAX_AGENTS; ++i)
    {
        Agent* agent = EE_ARRAY_AT(&agents, i, Agent);

        printf("Agent [%2zu]: ", i);
        ee_str_print(&agent->genome);
        printf("\n");
    }

    for (size_t iter = 0; iter < EEV_MAX_ITERS; ++iter)
    {
        ee_arena_reset(&best_gen_arena);

        for (size_t i = 0; i < EEV_MAX_AGENTS; ++i)
        {
            Agent* agent = EE_ARRAY_AT(&agents, i, Agent);
            s32 score = ee_str_lev(&target_str, &agent->genome);

            agent->score = score;
        }

        ee_array_sort(&agents, eev_score_cmp, EE_SORT_DEFAULT);

        for (size_t i = 0; i < EEV_BEST_AGENTS; ++i)
        {
            Agent* src = EE_ARRAY_AT(&agents, i, Agent);

            best_agents[i] = eev_agent_copy(src, &best_gen_alloc);
        }

        best_score = best_agents[0].score;

        if (best_score == 0)
        {
            last_iter = iter;
            break;
        }

        for (size_t i = 0; i < EEV_MAX_AGENTS; ++i)
        {
            s32 best_i = rand() % EEV_BEST_AGENTS;

            Agent best_0 = best_agents[best_i];
            Agent best_1 = best_agents[(best_i + 1) % EEV_BEST_AGENTS];

            eev_cross_and_mutate(&best_0, &best_1, EE_ARRAY_AT(&agents, i, Agent));
        }
        
        if (iter % 10 == 0)
        {
            printf("[%zu] Best Genome: ", iter);
            ee_str_print(&best_agents[0].genome);
            printf("\n[%zu] Best Score: %d\n", iter, best_agents[0].score);
        }
    }

    if (best_score == 0)
    {
        printf("Solved on iteration: %zu, Score: %d, Genome: ", last_iter, best_score);
        ee_str_print(&best_agents[0].genome);
        printf("\n");
    }
    else
    {
        printf("Does not solved\n");
    }
}
