#include "precompiled.h"
#pragma hdrstop

/*
=============================================================

  ALIAS MODELS

=============================================================
*/

typedef float vec4_t[ 4 ];

static	vec4_t	s_lerped[ MAX_VERTS ];

Vec3	shadevector;
Vec3	shadelight;

// precalculated dot products for quantized angles
#define SHADEDOT_QUANT 16
float	r_avertexnormal_dots[ SHADEDOT_QUANT ][ 256 ] = {

	{1.23f, 1.30f, 1.47f, 1.35f, 1.56f, 1.71f, 1.37f, 1.38f, 1.59f, 1.60f, 1.79f, 1.97f, 1.88f, 1.92f, 1.79f, 1.02f, 0.93f, 1.07f, 0.82f, 0.87f, 0.88f, 0.94f, 0.96f, 1.14f, 1.11f, 0.82f, 0.83f, 0.89f, 0.89f, 0.86f, 0.94f, 0.91f, 1.0f, 1.21f, 0.98f, 1.48f, 1.30f, 1.57f, 0.96f, 1.07f, 1.14f, 1.60f, 1.61f, 1.40f, 1.37f, 1.72f, 1.78f, 1.79f, 1.93f, 1.99f, 1.90f, 1.68f, 1.71f, 1.86f, 1.60f, 1.68f, 1.78f, 1.86f, 1.93f, 1.99f, 1.97f, 1.44f, 1.22f, 1.49f, 0.93f, 0.99f, 0.99f, 1.23f, 1.22f, 1.44f, 1.49f, 0.89f, 0.89f, 0.97f, 0.91f, 0.98f, 1.19f, 0.82f, 0.76f, 0.82f, 0.71f, 0.72f, 0.73f, 0.76f, 0.79f, 0.86f, 0.83f, 0.72f, 0.76f, 0.76f, 0.89f, 0.82f, 0.89f, 0.82f, 0.89f, 0.91f, 0.83f, 0.96f, 1.14f, 0.97f, 1.40f, 1.19f, 0.98f, 0.94f, 1.0f, 1.07f, 1.37f, 1.21f, 1.48f, 1.30f, 1.57f, 1.61f, 1.37f, 0.86f, 0.83f, 0.91f, 0.82f, 0.82f, 0.88f, 0.89f, 0.96f, 1.14f, 0.98f, 0.87f, 0.93f, 0.94f, 1.02f, 1.30f, 1.07f, 1.35f, 1.38f, 1.11f, 1.56f, 1.92f, 1.79f, 1.79f, 1.59f, 1.60f, 1.72f, 1.90f, 1.79f, 0.80f, 0.85f, 0.79f, 0.93f, 0.80f, 0.85f, 0.77f, 0.74f, 0.72f, 0.77f, 0.74f, 0.72f, 0.70f, 0.70f, 0.71f, 0.76f, 0.73f, 0.79f, 0.79f, 0.73f, 0.76f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, {1.26f, 1.26f, 1.48f, 1.23f, 1.50f, 1.71f, 1.14f, 1.19f, 1.38f, 1.46f, 1.64f, 1.94f, 1.87f, 1.84f, 1.71f, 1.02f, 0.92f, 1.0f, 0.79f, 0.85f, 0.84f, 0.91f, 0.90f, 0.98f, 0.99f, 0.77f, 0.77f, 0.83f, 0.82f, 0.79f, 0.86f, 0.84f, 0.92f, 0.99f, 0.91f, 1.24f, 1.03f, 1.33f, 0.88f, 0.94f, 0.97f, 1.41f, 1.39f, 1.18f, 1.11f, 1.51f, 1.61f, 1.59f, 1.80f, 1.91f, 1.76f, 1.54f, 1.65f, 1.76f, 1.70f, 1.70f, 1.85f, 1.85f, 1.97f, 1.99f, 1.93f, 1.28f, 1.09f, 1.39f, 0.92f, 0.97f, 0.99f, 1.18f, 1.26f, 1.52f, 1.48f, 0.83f, 0.85f, 0.90f, 0.88f, 0.93f, 1.0f, 0.77f, 0.73f, 0.78f, 0.72f, 0.71f, 0.74f, 0.75f, 0.79f, 0.86f, 0.81f, 0.75f, 0.81f, 0.79f, 0.96f, 0.88f, 0.94f, 0.86f, 0.93f, 0.92f, 0.85f, 1.08f, 1.33f, 1.05f, 1.55f, 1.31f, 1.01f, 1.05f, 1.27f, 1.31f, 1.60f, 1.47f, 1.70f, 1.54f, 1.76f, 1.76f, 1.57f, 0.93f, 0.90f, 0.99f, 0.88f, 0.88f, 0.95f, 0.97f, 1.11f, 1.39f, 1.20f, 0.92f, 0.97f, 1.01f, 1.10f, 1.39f, 1.22f, 1.51f, 1.58f, 1.32f, 1.64f, 1.97f, 1.85f, 1.91f, 1.77f, 1.74f, 1.88f, 1.99f, 1.91f, 0.79f, 0.86f, 0.80f, 0.94f, 0.84f, 0.88f, 0.74f, 0.74f, 0.71f, 0.82f, 0.77f, 0.76f, 0.70f, 0.73f, 0.72f, 0.73f, 0.70f, 0.74f, 0.85f, 0.77f, 0.82f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, {1.34f, 1.27f, 1.53f, 1.17f, 1.46f, 1.71f, 0.98f, 1.05f, 1.20f, 1.34f, 1.48f, 1.86f, 1.82f, 1.71f, 1.62f, 1.09f, 0.94f, 0.99f, 0.79f, 0.85f, 0.82f, 0.90f, 0.87f, 0.93f, 0.96f, 0.76f, 0.74f, 0.79f, 0.76f, 0.74f, 0.79f, 0.78f, 0.85f, 0.92f, 0.85f, 1.0f, 0.93f, 1.06f, 0.81f, 0.86f, 0.89f, 1.16f, 1.12f, 0.97f, 0.95f, 1.28f, 1.38f, 1.35f, 1.60f, 1.77f, 1.57f, 1.33f, 1.50f, 1.58f, 1.69f, 1.63f, 1.82f, 1.74f, 1.91f, 1.92f, 1.80f, 1.04f, 0.97f, 1.21f, 0.90f, 0.93f, 0.97f, 1.05f, 1.21f, 1.48f, 1.37f, 0.77f, 0.80f, 0.84f, 0.85f, 0.88f, 0.92f, 0.73f, 0.71f, 0.74f, 0.74f, 0.71f, 0.75f, 0.73f, 0.79f, 0.84f, 0.78f, 0.79f, 0.86f, 0.81f, 1.05f, 0.94f, 0.99f, 0.90f, 0.95f, 0.92f, 0.86f, 1.24f, 1.44f, 1.14f, 1.59f, 1.34f, 1.02f, 1.27f, 1.50f, 1.49f, 1.80f, 1.69f, 1.86f, 1.72f, 1.87f, 1.80f, 1.69f, 1.0f, 0.98f, 1.23f, 0.95f, 0.96f, 1.09f, 1.16f, 1.37f, 1.63f, 1.46f, 0.99f, 1.10f, 1.25f, 1.24f, 1.51f, 1.41f, 1.67f, 1.77f, 1.55f, 1.72f, 1.95f, 1.89f, 1.98f, 1.91f, 1.86f, 1.97f, 1.99f, 1.94f, 0.81f, 0.89f, 0.85f, 0.98f, 0.90f, 0.94f, 0.75f, 0.78f, 0.73f, 0.89f, 0.83f, 0.82f, 0.72f, 0.77f, 0.76f, 0.72f, 0.70f, 0.71f, 0.91f, 0.83f, 0.89f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, {1.46f, 1.34f, 1.60f, 1.16f, 1.46f, 1.71f, 0.94f, 0.99f, 1.05f, 1.26f, 1.33f, 1.74f, 1.76f, 1.57f, 1.54f, 1.23f, 0.98f, 1.05f, 0.83f, 0.89f, 0.84f, 0.92f, 0.87f, 0.91f, 0.96f, 0.78f, 0.74f, 0.79f, 0.72f, 0.72f, 0.75f, 0.76f, 0.80f, 0.88f, 0.83f, 0.94f, 0.87f, 0.95f, 0.76f, 0.80f, 0.82f, 0.97f, 0.96f, 0.89f, 0.88f, 1.08f, 1.11f, 1.10f, 1.37f, 1.59f, 1.37f, 1.07f, 1.27f, 1.34f, 1.57f, 1.45f, 1.69f, 1.55f, 1.77f, 1.79f, 1.60f, 0.93f, 0.90f, 0.99f, 0.86f, 0.87f, 0.93f, 0.96f, 1.07f, 1.35f, 1.18f, 0.73f, 0.76f, 0.77f, 0.81f, 0.82f, 0.85f, 0.70f, 0.71f, 0.72f, 0.78f, 0.73f, 0.77f, 0.73f, 0.79f, 0.82f, 0.76f, 0.83f, 0.90f, 0.84f, 1.18f, 0.98f, 1.03f, 0.92f, 0.95f, 0.90f, 0.86f, 1.32f, 1.45f, 1.15f, 1.53f, 1.27f, 0.99f, 1.42f, 1.65f, 1.58f, 1.93f, 1.83f, 1.94f, 1.81f, 1.88f, 1.74f, 1.70f, 1.19f, 1.17f, 1.44f, 1.11f, 1.15f, 1.36f, 1.41f, 1.61f, 1.81f, 1.67f, 1.22f, 1.34f, 1.50f, 1.42f, 1.65f, 1.61f, 1.82f, 1.91f, 1.75f, 1.80f, 1.89f, 1.89f, 1.98f, 1.99f, 1.94f, 1.98f, 1.92f, 1.87f, 0.86f, 0.95f, 0.92f, 1.14f, 0.98f, 1.03f, 0.79f, 0.84f, 0.77f, 0.97f, 0.90f, 0.89f, 0.76f, 0.82f, 0.82f, 0.74f, 0.72f, 0.71f, 0.98f, 0.89f, 0.97f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, {1.60f, 1.44f, 1.68f, 1.22f, 1.49f, 1.71f, 0.93f, 0.99f, 0.99f, 1.23f, 1.22f, 1.60f, 1.68f, 1.44f, 1.49f, 1.40f, 1.14f, 1.19f, 0.89f, 0.96f, 0.89f, 0.97f, 0.89f, 0.91f, 0.98f, 0.82f, 0.76f, 0.82f, 0.71f, 0.72f, 0.73f, 0.76f, 0.79f, 0.86f, 0.83f, 0.91f, 0.83f, 0.89f, 0.72f, 0.76f, 0.76f, 0.89f, 0.89f, 0.82f, 0.82f, 0.98f, 0.96f, 0.97f, 1.14f, 1.40f, 1.19f, 0.94f, 1.0f, 1.07f, 1.37f, 1.21f, 1.48f, 1.30f, 1.57f, 1.61f, 1.37f, 0.86f, 0.83f, 0.91f, 0.82f, 0.82f, 0.88f, 0.89f, 0.96f, 1.14f, 0.98f, 0.70f, 0.72f, 0.73f, 0.77f, 0.76f, 0.79f, 0.70f, 0.72f, 0.71f, 0.82f, 0.77f, 0.80f, 0.74f, 0.79f, 0.80f, 0.74f, 0.87f, 0.93f, 0.85f, 1.23f, 1.02f, 1.02f, 0.93f, 0.93f, 0.87f, 0.85f, 1.30f, 1.35f, 1.07f, 1.38f, 1.11f, 0.94f, 1.47f, 1.71f, 1.56f, 1.97f, 1.88f, 1.92f, 1.79f, 1.79f, 1.59f, 1.60f, 1.30f, 1.35f, 1.56f, 1.37f, 1.38f, 1.59f, 1.60f, 1.79f, 1.92f, 1.79f, 1.48f, 1.57f, 1.72f, 1.61f, 1.78f, 1.79f, 1.93f, 1.99f, 1.90f, 1.86f, 1.78f, 1.86f, 1.93f, 1.99f, 1.97f, 1.90f, 1.79f, 1.72f, 0.94f, 1.07f, 1.0f, 1.37f, 1.21f, 1.30f, 0.86f, 0.91f, 0.83f, 1.14f, 0.98f, 0.96f, 0.82f, 0.88f, 0.89f, 0.79f, 0.76f, 0.73f, 1.07f, 0.94f, 1.11f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, {1.74f, 1.57f, 1.76f, 1.33f, 1.54f, 1.71f, 0.94f, 1.05f, 0.99f, 1.26f, 1.16f, 1.46f, 1.60f, 1.34f, 1.46f, 1.59f, 1.37f, 1.37f, 0.97f, 1.11f, 0.96f, 1.10f, 0.95f, 0.94f, 1.08f, 0.89f, 0.82f, 0.88f, 0.72f, 0.76f, 0.75f, 0.80f, 0.80f, 0.88f, 0.87f, 0.91f, 0.83f, 0.87f, 0.72f, 0.76f, 0.74f, 0.83f, 0.84f, 0.78f, 0.79f, 0.96f, 0.89f, 0.92f, 0.98f, 1.23f, 1.05f, 0.86f, 0.92f, 0.95f, 1.11f, 0.98f, 1.22f, 1.03f, 1.34f, 1.42f, 1.14f, 0.79f, 0.77f, 0.84f, 0.78f, 0.76f, 0.82f, 0.82f, 0.89f, 0.97f, 0.90f, 0.70f, 0.71f, 0.71f, 0.73f, 0.72f, 0.74f, 0.73f, 0.76f, 0.72f, 0.86f, 0.81f, 0.82f, 0.76f, 0.79f, 0.77f, 0.73f, 0.90f, 0.95f, 0.86f, 1.18f, 1.03f, 0.98f, 0.92f, 0.90f, 0.83f, 0.84f, 1.19f, 1.17f, 0.98f, 1.15f, 0.97f, 0.89f, 1.42f, 1.65f, 1.44f, 1.93f, 1.83f, 1.81f, 1.67f, 1.61f, 1.36f, 1.41f, 1.32f, 1.45f, 1.58f, 1.57f, 1.53f, 1.74f, 1.70f, 1.88f, 1.94f, 1.81f, 1.69f, 1.77f, 1.87f, 1.79f, 1.89f, 1.92f, 1.98f, 1.99f, 1.98f, 1.89f, 1.65f, 1.80f, 1.82f, 1.91f, 1.94f, 1.75f, 1.61f, 1.50f, 1.07f, 1.34f, 1.27f, 1.60f, 1.45f, 1.55f, 0.93f, 0.99f, 0.90f, 1.35f, 1.18f, 1.07f, 0.87f, 0.93f, 0.96f, 0.85f, 0.82f, 0.77f, 1.15f, 0.99f, 1.27f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, {1.86f, 1.71f, 1.82f, 1.48f, 1.62f, 1.71f, 0.98f, 1.20f, 1.05f, 1.34f, 1.17f, 1.34f, 1.53f, 1.27f, 1.46f, 1.77f, 1.60f, 1.57f, 1.16f, 1.38f, 1.12f, 1.35f, 1.06f, 1.0f, 1.28f, 0.97f, 0.89f, 0.95f, 0.76f, 0.81f, 0.79f, 0.86f, 0.85f, 0.92f, 0.93f, 0.93f, 0.85f, 0.87f, 0.74f, 0.78f, 0.74f, 0.79f, 0.82f, 0.76f, 0.79f, 0.96f, 0.85f, 0.90f, 0.94f, 1.09f, 0.99f, 0.81f, 0.85f, 0.89f, 0.95f, 0.90f, 0.99f, 0.94f, 1.10f, 1.24f, 0.98f, 0.75f, 0.73f, 0.78f, 0.74f, 0.72f, 0.77f, 0.76f, 0.82f, 0.89f, 0.83f, 0.73f, 0.71f, 0.71f, 0.71f, 0.70f, 0.72f, 0.77f, 0.80f, 0.74f, 0.90f, 0.85f, 0.84f, 0.78f, 0.79f, 0.75f, 0.73f, 0.92f, 0.95f, 0.86f, 1.05f, 0.99f, 0.94f, 0.90f, 0.86f, 0.79f, 0.81f, 1.0f, 0.98f, 0.91f, 0.96f, 0.89f, 0.83f, 1.27f, 1.50f, 1.23f, 1.80f, 1.69f, 1.63f, 1.46f, 1.37f, 1.09f, 1.16f, 1.24f, 1.44f, 1.49f, 1.69f, 1.59f, 1.80f, 1.69f, 1.87f, 1.86f, 1.72f, 1.82f, 1.91f, 1.94f, 1.92f, 1.95f, 1.99f, 1.98f, 1.91f, 1.97f, 1.89f, 1.51f, 1.72f, 1.67f, 1.77f, 1.86f, 1.55f, 1.41f, 1.25f, 1.33f, 1.58f, 1.50f, 1.80f, 1.63f, 1.74f, 1.04f, 1.21f, 0.97f, 1.48f, 1.37f, 1.21f, 0.93f, 0.97f, 1.05f, 0.92f, 0.88f, 0.84f, 1.14f, 1.02f, 1.34f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, {1.94f, 1.84f, 1.87f, 1.64f, 1.71f, 1.71f, 1.14f, 1.38f, 1.19f, 1.46f, 1.23f, 1.26f, 1.48f, 1.26f, 1.50f, 1.91f, 1.80f, 1.76f, 1.41f, 1.61f, 1.39f, 1.59f, 1.33f, 1.24f, 1.51f, 1.18f, 0.97f, 1.11f, 0.82f, 0.88f, 0.86f, 0.94f, 0.92f, 0.99f, 1.03f, 0.98f, 0.91f, 0.90f, 0.79f, 0.84f, 0.77f, 0.79f, 0.84f, 0.77f, 0.83f, 0.99f, 0.85f, 0.91f, 0.92f, 1.02f, 1.0f, 0.79f, 0.80f, 0.86f, 0.88f, 0.84f, 0.92f, 0.88f, 0.97f, 1.10f, 0.94f, 0.74f, 0.71f, 0.74f, 0.72f, 0.70f, 0.73f, 0.72f, 0.76f, 0.82f, 0.77f, 0.77f, 0.73f, 0.74f, 0.71f, 0.70f, 0.73f, 0.83f, 0.85f, 0.78f, 0.92f, 0.88f, 0.86f, 0.81f, 0.79f, 0.74f, 0.75f, 0.92f, 0.93f, 0.85f, 0.96f, 0.94f, 0.88f, 0.86f, 0.81f, 0.75f, 0.79f, 0.93f, 0.90f, 0.85f, 0.88f, 0.82f, 0.77f, 1.05f, 1.27f, 0.99f, 1.60f, 1.47f, 1.39f, 1.20f, 1.11f, 0.95f, 0.97f, 1.08f, 1.33f, 1.31f, 1.70f, 1.55f, 1.76f, 1.57f, 1.76f, 1.70f, 1.54f, 1.85f, 1.97f, 1.91f, 1.99f, 1.97f, 1.99f, 1.91f, 1.77f, 1.88f, 1.85f, 1.39f, 1.64f, 1.51f, 1.58f, 1.74f, 1.32f, 1.22f, 1.01f, 1.54f, 1.76f, 1.65f, 1.93f, 1.70f, 1.85f, 1.28f, 1.39f, 1.09f, 1.52f, 1.48f, 1.26f, 0.97f, 0.99f, 1.18f, 1.0f, 0.93f, 0.90f, 1.05f, 1.01f, 1.31f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, {1.97f, 1.92f, 1.88f, 1.79f, 1.79f, 1.71f, 1.37f, 1.59f, 1.38f, 1.60f, 1.35f, 1.23f, 1.47f, 1.30f, 1.56f, 1.99f, 1.93f, 1.90f, 1.60f, 1.78f, 1.61f, 1.79f, 1.57f, 1.48f, 1.72f, 1.40f, 1.14f, 1.37f, 0.89f, 0.96f, 0.94f, 1.07f, 1.0f, 1.21f, 1.30f, 1.14f, 0.98f, 0.96f, 0.86f, 0.91f, 0.83f, 0.82f, 0.88f, 0.82f, 0.89f, 1.11f, 0.87f, 0.94f, 0.93f, 1.02f, 1.07f, 0.80f, 0.79f, 0.85f, 0.82f, 0.80f, 0.87f, 0.85f, 0.93f, 1.02f, 0.93f, 0.77f, 0.72f, 0.74f, 0.71f, 0.70f, 0.70f, 0.71f, 0.72f, 0.77f, 0.74f, 0.82f, 0.76f, 0.79f, 0.72f, 0.73f, 0.76f, 0.89f, 0.89f, 0.82f, 0.93f, 0.91f, 0.86f, 0.83f, 0.79f, 0.73f, 0.76f, 0.91f, 0.89f, 0.83f, 0.89f, 0.89f, 0.82f, 0.82f, 0.76f, 0.72f, 0.76f, 0.86f, 0.83f, 0.79f, 0.82f, 0.76f, 0.73f, 0.94f, 1.0f, 0.91f, 1.37f, 1.21f, 1.14f, 0.98f, 0.96f, 0.88f, 0.89f, 0.96f, 1.14f, 1.07f, 1.60f, 1.40f, 1.61f, 1.37f, 1.57f, 1.48f, 1.30f, 1.78f, 1.93f, 1.79f, 1.99f, 1.92f, 1.90f, 1.79f, 1.59f, 1.72f, 1.79f, 1.30f, 1.56f, 1.35f, 1.38f, 1.60f, 1.11f, 1.07f, 0.94f, 1.68f, 1.86f, 1.71f, 1.97f, 1.68f, 1.86f, 1.44f, 1.49f, 1.22f, 1.44f, 1.49f, 1.22f, 0.99f, 0.99f, 1.23f, 1.19f, 0.98f, 0.97f, 0.97f, 0.98f, 1.19f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, {1.94f, 1.97f, 1.87f, 1.91f, 1.85f, 1.71f, 1.60f, 1.77f, 1.58f, 1.74f, 1.51f, 1.26f, 1.48f, 1.39f, 1.64f, 1.99f, 1.97f, 1.99f, 1.70f, 1.85f, 1.76f, 1.91f, 1.76f, 1.70f, 1.88f, 1.55f, 1.33f, 1.57f, 0.96f, 1.08f, 1.05f, 1.31f, 1.27f, 1.47f, 1.54f, 1.39f, 1.20f, 1.11f, 0.93f, 0.99f, 0.90f, 0.88f, 0.95f, 0.88f, 0.97f, 1.32f, 0.92f, 1.01f, 0.97f, 1.10f, 1.22f, 0.84f, 0.80f, 0.88f, 0.79f, 0.79f, 0.85f, 0.86f, 0.92f, 1.02f, 0.94f, 0.82f, 0.76f, 0.77f, 0.72f, 0.73f, 0.70f, 0.72f, 0.71f, 0.74f, 0.74f, 0.88f, 0.81f, 0.85f, 0.75f, 0.77f, 0.82f, 0.94f, 0.93f, 0.86f, 0.92f, 0.92f, 0.86f, 0.85f, 0.79f, 0.74f, 0.79f, 0.88f, 0.85f, 0.81f, 0.82f, 0.83f, 0.77f, 0.78f, 0.73f, 0.71f, 0.75f, 0.79f, 0.77f, 0.74f, 0.77f, 0.73f, 0.70f, 0.86f, 0.92f, 0.84f, 1.14f, 0.99f, 0.98f, 0.91f, 0.90f, 0.84f, 0.83f, 0.88f, 0.97f, 0.94f, 1.41f, 1.18f, 1.39f, 1.11f, 1.33f, 1.24f, 1.03f, 1.61f, 1.80f, 1.59f, 1.91f, 1.84f, 1.76f, 1.64f, 1.38f, 1.51f, 1.71f, 1.26f, 1.50f, 1.23f, 1.19f, 1.46f, 0.99f, 1.0f, 0.91f, 1.70f, 1.85f, 1.65f, 1.93f, 1.54f, 1.76f, 1.52f, 1.48f, 1.26f, 1.28f, 1.39f, 1.09f, 0.99f, 0.97f, 1.18f, 1.31f, 1.01f, 1.05f, 0.90f, 0.93f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, {1.86f, 1.95f, 1.82f, 1.98f, 1.89f, 1.71f, 1.80f, 1.91f, 1.77f, 1.86f, 1.67f, 1.34f, 1.53f, 1.51f, 1.72f, 1.92f, 1.91f, 1.99f, 1.69f, 1.82f, 1.80f, 1.94f, 1.87f, 1.86f, 1.97f, 1.59f, 1.44f, 1.69f, 1.05f, 1.24f, 1.27f, 1.49f, 1.50f, 1.69f, 1.72f, 1.63f, 1.46f, 1.37f, 1.0f, 1.23f, 0.98f, 0.95f, 1.09f, 0.96f, 1.16f, 1.55f, 0.99f, 1.25f, 1.10f, 1.24f, 1.41f, 0.90f, 0.85f, 0.94f, 0.79f, 0.81f, 0.85f, 0.89f, 0.94f, 1.09f, 0.98f, 0.89f, 0.82f, 0.83f, 0.74f, 0.77f, 0.72f, 0.76f, 0.73f, 0.75f, 0.78f, 0.94f, 0.86f, 0.91f, 0.79f, 0.83f, 0.89f, 0.99f, 0.95f, 0.90f, 0.90f, 0.92f, 0.84f, 0.86f, 0.79f, 0.75f, 0.81f, 0.85f, 0.80f, 0.78f, 0.76f, 0.77f, 0.73f, 0.74f, 0.71f, 0.71f, 0.73f, 0.74f, 0.74f, 0.71f, 0.76f, 0.72f, 0.70f, 0.79f, 0.85f, 0.78f, 0.98f, 0.92f, 0.93f, 0.85f, 0.87f, 0.82f, 0.79f, 0.81f, 0.89f, 0.86f, 1.16f, 0.97f, 1.12f, 0.95f, 1.06f, 1.0f, 0.93f, 1.38f, 1.60f, 1.35f, 1.77f, 1.71f, 1.57f, 1.48f, 1.20f, 1.28f, 1.62f, 1.27f, 1.46f, 1.17f, 1.05f, 1.34f, 0.96f, 0.99f, 0.90f, 1.63f, 1.74f, 1.50f, 1.80f, 1.33f, 1.58f, 1.48f, 1.37f, 1.21f, 1.04f, 1.21f, 0.97f, 0.97f, 0.93f, 1.05f, 1.34f, 1.02f, 1.14f, 0.84f, 0.88f, 0.92f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, {1.74f, 1.89f, 1.76f, 1.98f, 1.89f, 1.71f, 1.93f, 1.99f, 1.91f, 1.94f, 1.82f, 1.46f, 1.60f, 1.65f, 1.80f, 1.79f, 1.77f, 1.92f, 1.57f, 1.69f, 1.74f, 1.87f, 1.88f, 1.94f, 1.98f, 1.53f, 1.45f, 1.70f, 1.18f, 1.32f, 1.42f, 1.58f, 1.65f, 1.83f, 1.81f, 1.81f, 1.67f, 1.61f, 1.19f, 1.44f, 1.17f, 1.11f, 1.36f, 1.15f, 1.41f, 1.75f, 1.22f, 1.50f, 1.34f, 1.42f, 1.61f, 0.98f, 0.92f, 1.03f, 0.83f, 0.86f, 0.89f, 0.95f, 0.98f, 1.23f, 1.14f, 0.97f, 0.89f, 0.90f, 0.78f, 0.82f, 0.76f, 0.82f, 0.77f, 0.79f, 0.84f, 0.98f, 0.90f, 0.98f, 0.83f, 0.89f, 0.97f, 1.03f, 0.95f, 0.92f, 0.86f, 0.90f, 0.82f, 0.86f, 0.79f, 0.77f, 0.84f, 0.81f, 0.76f, 0.76f, 0.72f, 0.73f, 0.70f, 0.72f, 0.71f, 0.73f, 0.73f, 0.72f, 0.74f, 0.71f, 0.78f, 0.74f, 0.72f, 0.75f, 0.80f, 0.76f, 0.94f, 0.88f, 0.91f, 0.83f, 0.87f, 0.84f, 0.79f, 0.76f, 0.82f, 0.80f, 0.97f, 0.89f, 0.96f, 0.88f, 0.95f, 0.94f, 0.87f, 1.11f, 1.37f, 1.10f, 1.59f, 1.57f, 1.37f, 1.33f, 1.05f, 1.08f, 1.54f, 1.34f, 1.46f, 1.16f, 0.99f, 1.26f, 0.96f, 1.05f, 0.92f, 1.45f, 1.55f, 1.27f, 1.60f, 1.07f, 1.34f, 1.35f, 1.18f, 1.07f, 0.93f, 0.99f, 0.90f, 0.93f, 0.87f, 0.96f, 1.27f, 0.99f, 1.15f, 0.77f, 0.82f, 0.85f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, {1.60f, 1.78f, 1.68f, 1.93f, 1.86f, 1.71f, 1.97f, 1.99f, 1.99f, 1.97f, 1.93f, 1.60f, 1.68f, 1.78f, 1.86f, 1.61f, 1.57f, 1.79f, 1.37f, 1.48f, 1.59f, 1.72f, 1.79f, 1.92f, 1.90f, 1.38f, 1.35f, 1.60f, 1.23f, 1.30f, 1.47f, 1.56f, 1.71f, 1.88f, 1.79f, 1.92f, 1.79f, 1.79f, 1.30f, 1.56f, 1.35f, 1.37f, 1.59f, 1.38f, 1.60f, 1.90f, 1.48f, 1.72f, 1.57f, 1.61f, 1.79f, 1.21f, 1.0f, 1.30f, 0.89f, 0.94f, 0.96f, 1.07f, 1.14f, 1.40f, 1.37f, 1.14f, 0.96f, 0.98f, 0.82f, 0.88f, 0.82f, 0.89f, 0.83f, 0.86f, 0.91f, 1.02f, 0.93f, 1.07f, 0.87f, 0.94f, 1.11f, 1.02f, 0.93f, 0.93f, 0.82f, 0.87f, 0.80f, 0.85f, 0.79f, 0.80f, 0.85f, 0.77f, 0.72f, 0.74f, 0.71f, 0.70f, 0.70f, 0.71f, 0.72f, 0.77f, 0.74f, 0.72f, 0.76f, 0.73f, 0.82f, 0.79f, 0.76f, 0.73f, 0.79f, 0.76f, 0.93f, 0.86f, 0.91f, 0.83f, 0.89f, 0.89f, 0.82f, 0.72f, 0.76f, 0.76f, 0.89f, 0.82f, 0.89f, 0.82f, 0.89f, 0.91f, 0.83f, 0.96f, 1.14f, 0.97f, 1.40f, 1.44f, 1.19f, 1.22f, 0.99f, 0.98f, 1.49f, 1.44f, 1.49f, 1.22f, 0.99f, 1.23f, 0.98f, 1.19f, 0.97f, 1.21f, 1.30f, 1.0f, 1.37f, 0.94f, 1.07f, 1.14f, 0.98f, 0.96f, 0.86f, 0.91f, 0.83f, 0.88f, 0.82f, 0.89f, 1.11f, 0.94f, 1.07f, 0.73f, 0.76f, 0.79f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, {1.46f, 1.65f, 1.60f, 1.82f, 1.80f, 1.71f, 1.93f, 1.91f, 1.99f, 1.94f, 1.98f, 1.74f, 1.76f, 1.89f, 1.89f, 1.42f, 1.34f, 1.61f, 1.11f, 1.22f, 1.36f, 1.50f, 1.61f, 1.81f, 1.75f, 1.15f, 1.17f, 1.41f, 1.18f, 1.19f, 1.42f, 1.44f, 1.65f, 1.83f, 1.67f, 1.94f, 1.81f, 1.88f, 1.32f, 1.58f, 1.45f, 1.57f, 1.74f, 1.53f, 1.70f, 1.98f, 1.69f, 1.87f, 1.77f, 1.79f, 1.92f, 1.45f, 1.27f, 1.55f, 0.97f, 1.07f, 1.11f, 1.34f, 1.37f, 1.59f, 1.60f, 1.35f, 1.07f, 1.18f, 0.86f, 0.93f, 0.87f, 0.96f, 0.90f, 0.93f, 0.99f, 1.03f, 0.95f, 1.15f, 0.90f, 0.99f, 1.27f, 0.98f, 0.90f, 0.92f, 0.78f, 0.83f, 0.77f, 0.84f, 0.79f, 0.82f, 0.86f, 0.73f, 0.71f, 0.73f, 0.72f, 0.70f, 0.73f, 0.72f, 0.76f, 0.81f, 0.76f, 0.76f, 0.82f, 0.77f, 0.89f, 0.85f, 0.82f, 0.75f, 0.80f, 0.80f, 0.94f, 0.88f, 0.94f, 0.87f, 0.95f, 0.96f, 0.88f, 0.72f, 0.74f, 0.76f, 0.83f, 0.78f, 0.84f, 0.79f, 0.87f, 0.91f, 0.83f, 0.89f, 0.98f, 0.92f, 1.23f, 1.34f, 1.05f, 1.16f, 0.99f, 0.96f, 1.46f, 1.57f, 1.54f, 1.33f, 1.05f, 1.26f, 1.08f, 1.37f, 1.10f, 0.98f, 1.03f, 0.92f, 1.14f, 0.86f, 0.95f, 0.97f, 0.90f, 0.89f, 0.79f, 0.84f, 0.77f, 0.82f, 0.76f, 0.82f, 0.97f, 0.89f, 0.98f, 0.71f, 0.72f, 0.74f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, {1.34f, 1.51f, 1.53f, 1.67f, 1.72f, 1.71f, 1.80f, 1.77f, 1.91f, 1.86f, 1.98f, 1.86f, 1.82f, 1.95f, 1.89f, 1.24f, 1.10f, 1.41f, 0.95f, 0.99f, 1.09f, 1.25f, 1.37f, 1.63f, 1.55f, 0.96f, 0.98f, 1.16f, 1.05f, 1.0f, 1.27f, 1.23f, 1.50f, 1.69f, 1.46f, 1.86f, 1.72f, 1.87f, 1.24f, 1.49f, 1.44f, 1.69f, 1.80f, 1.59f, 1.69f, 1.97f, 1.82f, 1.94f, 1.91f, 1.92f, 1.99f, 1.63f, 1.50f, 1.74f, 1.16f, 1.33f, 1.38f, 1.58f, 1.60f, 1.77f, 1.80f, 1.48f, 1.21f, 1.37f, 0.90f, 0.97f, 0.93f, 1.05f, 0.97f, 1.04f, 1.21f, 0.99f, 0.95f, 1.14f, 0.92f, 1.02f, 1.34f, 0.94f, 0.86f, 0.90f, 0.74f, 0.79f, 0.75f, 0.81f, 0.79f, 0.84f, 0.86f, 0.71f, 0.71f, 0.73f, 0.76f, 0.73f, 0.77f, 0.74f, 0.80f, 0.85f, 0.78f, 0.81f, 0.89f, 0.84f, 0.97f, 0.92f, 0.88f, 0.79f, 0.85f, 0.86f, 0.98f, 0.92f, 1.0f, 0.93f, 1.06f, 1.12f, 0.95f, 0.74f, 0.74f, 0.78f, 0.79f, 0.76f, 0.82f, 0.79f, 0.87f, 0.93f, 0.85f, 0.85f, 0.94f, 0.90f, 1.09f, 1.27f, 0.99f, 1.17f, 1.05f, 0.96f, 1.46f, 1.71f, 1.62f, 1.48f, 1.20f, 1.34f, 1.28f, 1.57f, 1.35f, 0.90f, 0.94f, 0.85f, 0.98f, 0.81f, 0.89f, 0.89f, 0.83f, 0.82f, 0.75f, 0.78f, 0.73f, 0.77f, 0.72f, 0.76f, 0.89f, 0.83f, 0.91f, 0.71f, 0.70f, 0.72f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, {1.26f, 1.39f, 1.48f, 1.51f, 1.64f, 1.71f, 1.60f, 1.58f, 1.77f, 1.74f, 1.91f, 1.94f, 1.87f, 1.97f, 1.85f, 1.10f, 0.97f, 1.22f, 0.88f, 0.92f, 0.95f, 1.01f, 1.11f, 1.39f, 1.32f, 0.88f, 0.90f, 0.97f, 0.96f, 0.93f, 1.05f, 0.99f, 1.27f, 1.47f, 1.20f, 1.70f, 1.54f, 1.76f, 1.08f, 1.31f, 1.33f, 1.70f, 1.76f, 1.55f, 1.57f, 1.88f, 1.85f, 1.91f, 1.97f, 1.99f, 1.99f, 1.70f, 1.65f, 1.85f, 1.41f, 1.54f, 1.61f, 1.76f, 1.80f, 1.91f, 1.93f, 1.52f, 1.26f, 1.48f, 0.92f, 0.99f, 0.97f, 1.18f, 1.09f, 1.28f, 1.39f, 0.94f, 0.93f, 1.05f, 0.92f, 1.01f, 1.31f, 0.88f, 0.81f, 0.86f, 0.72f, 0.75f, 0.74f, 0.79f, 0.79f, 0.86f, 0.85f, 0.71f, 0.73f, 0.75f, 0.82f, 0.77f, 0.83f, 0.78f, 0.85f, 0.88f, 0.81f, 0.88f, 0.97f, 0.90f, 1.18f, 1.0f, 0.93f, 0.86f, 0.92f, 0.94f, 1.14f, 0.99f, 1.24f, 1.03f, 1.33f, 1.39f, 1.11f, 0.79f, 0.77f, 0.84f, 0.79f, 0.77f, 0.84f, 0.83f, 0.90f, 0.98f, 0.91f, 0.85f, 0.92f, 0.91f, 1.02f, 1.26f, 1.0f, 1.23f, 1.19f, 0.99f, 1.50f, 1.84f, 1.71f, 1.64f, 1.38f, 1.46f, 1.51f, 1.76f, 1.59f, 0.84f, 0.88f, 0.80f, 0.94f, 0.79f, 0.86f, 0.82f, 0.77f, 0.76f, 0.74f, 0.74f, 0.71f, 0.73f, 0.70f, 0.72f, 0.82f, 0.77f, 0.85f, 0.74f, 0.70f, 0.73f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}

};

float	* shadedots = r_avertexnormal_dots[ 0 ];

void Renderer::GL_LerpVerts( int nverts, dtrivertx_t * v, dtrivertx_t * ov, dtrivertx_t * verts, float * lerp, Vec3 & move, Vec3 & frontv, Vec3 & backv ) {

	if( currententity->flags &( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM ) ) {

		for( int i = 0; i < nverts; i++, v++, ov++, lerp+= 4 ) {

			Vec3 normal = Common::bytedirs[ verts[ i ].lightnormalindex ];

			lerp[ 0 ] = move[ 0 ] + ov->v[ 0 ] * backv[ 0 ] + v->v[ 0 ] * frontv[ 0 ] + normal[ 0 ] * POWERSUIT_SCALE;
			lerp[ 1 ] = move[ 1 ] + ov->v[ 1 ] * backv[ 1 ] + v->v[ 1 ] * frontv[ 1 ] + normal[ 1 ] * POWERSUIT_SCALE;
			lerp[ 2 ] = move[ 2 ] + ov->v[ 2 ] * backv[ 2 ] + v->v[ 2 ] * frontv[ 2 ] + normal[ 2 ] * POWERSUIT_SCALE; 
		}
	} else {

		for( int i = 0; i < nverts; i++, v++, ov++, lerp += 4 ) {

			lerp[ 0 ] = move[ 0 ] + ov->v[ 0 ] * backv[ 0 ] + v->v[ 0 ] * frontv[ 0 ];
			lerp[ 1 ] = move[ 1 ] + ov->v[ 1 ] * backv[ 1 ] + v->v[ 1 ] * frontv[ 1 ];
			lerp[ 2 ] = move[ 2 ] + ov->v[ 2 ] * backv[ 2 ] + v->v[ 2 ] * frontv[ 2 ];
		}
	}

}

/*
=============
GL_DrawAliasFrameLerp

interpolates between two frames and origins
FIXME: batch lerp all vertexes
=============
*/
void Renderer::GL_DrawAliasFrameLerp( dmdl_t * paliashdr, float backlerp ) {

	float 			l;
	daliasframe_t	* frame, * oldframe;
	dtrivertx_t		* v, * ov, * verts;
	int				* order;
	int				count;
	float			frontlerp;
	float			alpha;
	Vec3			move, delta, vectors[ 3 ];
	Vec3			frontv, backv;
	int				index_xyz;
	float			* lerp;

	frame =( daliasframe_t * )( ( byte * )paliashdr + paliashdr->ofs_frames  + currententity->frame * paliashdr->framesize );
	verts = v = frame->verts;

	oldframe =( daliasframe_t * )( ( byte * )paliashdr + paliashdr->ofs_frames + currententity->oldframe * paliashdr->framesize );
	ov = oldframe->verts;

	order =( int * )( ( byte * )paliashdr + paliashdr->ofs_glcmds );

	if( currententity->flags & RF_TRANSLUCENT ) alpha = currententity->alpha;
	else alpha = 1.0f;

	//if( currententity->flags &( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM ) ) glDisable( GL_TEXTURE_2D );

	frontlerp = 1.0f - backlerp;

	// move should be the delta back to the previous frame * backlerp
	delta = currententity->oldorigin - currententity->origin;
	currententity->angles.AngleVectors( &vectors[ 0 ], &vectors[ 1 ], &vectors[ 2 ] );

	move[ 0 ] =  delta * vectors[ 0 ];	// forward
	move[ 1 ] = -( delta * vectors[ 1 ] );	// left
	move[ 2 ] =  delta * vectors[ 2 ];	// up

	move += Vec3( oldframe->translate[ 0 ], oldframe->translate[ 1 ], oldframe->translate[ 2 ] );

	for( int i = 0; i < 3; i++ )  move[ i ] = backlerp* move[ i ] + frontlerp* frame->translate[ i ];

	for( int i = 0; i < 3; i++ ) {

		frontv[ i ]	= frontlerp * frame->scale[ i ];
		backv[ i ]	= backlerp * oldframe->scale[ i ];
	}

	lerp = s_lerped[ 0 ];

	GL_LerpVerts( paliashdr->num_xyz, v, ov, verts, lerp, move, frontv, backv );

	if( gl_vertex_arrays.GetBool( ) ) {

		float colorArray[ MAX_VERTS * 4 ];

		glEnableClientState( GL_VERTEX_ARRAY );
		glVertexPointer( 3, GL_FLOAT, 16, s_lerped );	// padded for SIMD

		if( currententity->flags &( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM ) ) glColor4f( shadelight[ 0 ], shadelight[ 1 ], shadelight[ 2 ], alpha );
		else {

			glEnableClientState( GL_COLOR_ARRAY );
			glColorPointer( 3, GL_FLOAT, 0, colorArray );

			//
			// pre light everything
			//
			for( int i = 0; i < paliashdr->num_xyz; i++ ) {

				float l = shadedots[ verts[ i ].lightnormalindex ];

				colorArray[ i * 3 + 0 ] = l * shadelight[ 0 ];
				colorArray[ i * 3 + 1 ] = l * shadelight[ 1 ];
				colorArray[ i * 3 + 2 ] = l * shadelight[ 2 ];
			}
		}

		GLImports::glLockArraysEXT( 0, paliashdr->num_xyz );

		while( 1 ) {

			// get the vertex count and primitive type
			count = * order++;

			if( !count ) break;		// done
			if( count < 0 ) {

				count = -count;
				glBegin( GL_TRIANGLE_FAN );
			} else glBegin( GL_TRIANGLE_STRIP );

			if( currententity->flags &( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM ) ) {

				do {

					index_xyz = order[ 2 ];
					order += 3;

					glVertex3fv( s_lerped[ index_xyz ] );

				} while( --count );
			} else {

				do {
					// texture coordinates come from the draw list
					glTexCoord2f( ( ( float * )order )[ 0 ], ( ( float * )order )[ 1 ] );
					index_xyz = order[ 2 ];

					order += 3;

					glArrayElement( index_xyz );

				} while( --count );
			}

			glEnd( );
		}

		GLImports::glUnlockArraysEXT( );

	} else {

		while( 1 ) {

			// get the vertex count and primitive type
			count = * order++;

			if( !count ) break;		// done
			if( count < 0 ) {

				count = -count;
				glBegin( GL_TRIANGLE_FAN );
			} else glBegin( GL_TRIANGLE_STRIP );

			if( currententity->flags &( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE ) ) {

				do {

					index_xyz = order[ 2 ];
					order += 3;

					glColor4f( shadelight[ 0 ], shadelight[ 1 ], shadelight[ 2 ], alpha );
					glVertex3fv( s_lerped[ index_xyz ] );

				} while( --count );

			} else {

				do {
					// texture coordinates come from the draw list
					glTexCoord2f( ( ( float * )order )[ 0 ], ( ( float * )order )[ 1 ] );
					index_xyz = order[ 2 ];
					order += 3;

					// normals and vertexes come from the frame list
					l = shadedots[ verts[ index_xyz ].lightnormalindex ];
					
					glColor4f( l * shadelight[ 0 ], l * shadelight[ 1 ], l * shadelight[ 2 ], alpha );
					glVertex3fv( s_lerped[ index_xyz ] );
				} while( --count );
			}

			glEnd( );
		}
	}

	//if( currententity->flags &( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM ) ) glEnable( GL_TEXTURE_2D );
}

/*
=============
GL_DrawAliasShadow
=============
*/
void Renderer::GL_DrawAliasShadow( dmdl_t * paliashdr, int posenum ) {

	dtrivertx_t * 		verts;
	int * 				order;
	Vec3				point;
	float				height		= 0.0f;
	float				lheight;
	int					count;
	daliasframe_t * 		frame;

	lheight = currententity->origin[ 2 ] - lightspot[ 2 ];

	frame =( daliasframe_t * )( ( byte * )paliashdr + paliashdr->ofs_frames  + currententity->frame * paliashdr->framesize );
	verts = frame->verts;

	order =( int * )( ( byte * )paliashdr + paliashdr->ofs_glcmds );

	height = -lheight + 1.0f;

	while( 1 ) {

		// get the vertex count and primitive type
		count = * order++;
		if( !count ) break;		// done
		if( count < 0 ) {

			count = -count;
			glBegin( GL_TRIANGLE_FAN );

		} else glBegin( GL_TRIANGLE_STRIP );

		do {
			// normals and vertexes come from the frame list

			memcpy( point.ToFloatPtr( ), s_lerped[ order[ 2 ] ], sizeof( point )  );

			point[ 0 ] -= shadevector[ 0 ] * ( point[ 2 ] + lheight );
			point[ 1 ] -= shadevector[ 1 ] * ( point[ 2 ] + lheight );
			point[ 2 ] = height;
			glVertex3fv( point.ToFloatPtr( ) );

			order += 3;

		} while( --count );

		glEnd( );
	}	
}

/*
* * R_CullAliasModel
*/
bool Renderer::CullAliasModel( Vec3 bbox[ 8 ], entity_t * e ) {

	Vec3			mins, maxs;
	dmdl_t			* paliashdr;
	Vec3			vectors[ 3 ];
	Vec3			thismins, oldmins, thismaxs, oldmaxs;
	daliasframe_t	* pframe, * poldframe;
	Vec3			angles;

	paliashdr =( dmdl_t * )currentmodel->extradata;

	if( ( e->frame >= paliashdr->num_frames ) ||( e->frame < 0 ) ) {

		Common::Com_Printf( "R_CullAliasModel %s: no such frame %d\n", currentmodel->name.c_str( ), e->frame );
		e->frame = 0;
	}
	if( ( e->oldframe >= paliashdr->num_frames ) ||( e->oldframe < 0 ) ) {

		Common::Com_Printf( "R_CullAliasModel %s: no such oldframe %d\n", currentmodel->name.c_str( ), e->oldframe );
		e->oldframe = 0;
	}

	pframe =( daliasframe_t * )( ( byte * ) paliashdr + paliashdr->ofs_frames + e->frame * paliashdr->framesize );

	poldframe =( daliasframe_t * )( ( byte * ) paliashdr + paliashdr->ofs_frames + e->oldframe * paliashdr->framesize );

	/*
	* * compute axially aligned mins and maxs
	*/
	if( pframe == poldframe ) {

		for( int i = 0; i < 3; i++ ) {

			mins[ i ] = pframe->translate[ i ];
			maxs[ i ] = mins[ i ] + pframe->scale[ i ] * 255.0f;
		}

	} else {

		for( int i = 0; i < 3; i++ ) {

			thismins[ i ] = pframe->translate[ i ];
			thismaxs[ i ] = thismins[ i ] + pframe->scale[ i ] * 255.0f;

			oldmins[ i ]  = poldframe->translate[ i ];
			oldmaxs[ i ]  = oldmins[ i ] + poldframe->scale[ i ] * 255.0f;

			if( thismins[ i ] < oldmins[ i ] ) mins[ i ] = thismins[ i ];
			else mins[ i ] = oldmins[ i ];

			if( thismaxs[ i ] > oldmaxs[ i ] ) maxs[ i ] = thismaxs[ i ];
			else maxs[ i ] = oldmaxs[ i ];
		}
	}

	/*
	* * compute a full bounding box
	*/
	for( int i = 0; i < 8; i++ ) {

		Vec3   tmp;

		if( i & 1 ) tmp[ 0 ] = mins[ 0 ];
		else tmp[ 0 ] = maxs[ 0 ];

		if( i & 2 ) tmp[ 1 ] = mins[ 1 ];
		else tmp[ 1 ] = maxs[ 1 ];

		if( i & 4 ) tmp[ 2 ] = mins[ 2 ];
		else tmp[ 2 ] = maxs[ 2 ];

		bbox[ i ] = tmp;
	}

	/*
	* * rotate the bounding box
	*/
	angles = e->angles;
	angles[ YAW ] = -angles[ YAW ];
	angles.AngleVectors( &vectors[ 0 ], &vectors[ 1 ], &vectors[ 2 ] );

	for( int i = 0; i < 8; i++ ) {

		Vec3 tmp = bbox[ i ];

		bbox[ i ][ 0 ] =  vectors[ 0 ] * tmp;
		bbox[ i ][ 1 ] = -( vectors[ 1 ] * tmp );
		bbox[ i ][ 2 ] =  vectors[ 2 ] * tmp;

		bbox[ i ] = e->origin + bbox[ i ];
	}

	int aggregatemask = ~0;

	for( int p = 0; p < 8; p++ ) {

		int mask = 0;

		for( int f = 0; f < 4; f++ ) {

			float dp = frustum[ f ].Normal( ) * bbox[ p ];

			if( ( dp - frustum[ f ].Dist( ) ) < 0.0f ) mask |=( 1 << f );
		}

		aggregatemask &= mask;
	}

	if( aggregatemask ) return true;

	return false;
}

/*
=================
R_DrawAliasModel

=================
*/
void Renderer::DrawAliasModel( entity_t * e ) {

	dmdl_t		* paliashdr;
	float		an;
	Vec3		bbox[ 8 ];
	TextureBase	* skin;

	if( !( e->flags & RF_WEAPONMODEL ) ) {

		if( CullAliasModel( bbox, e ) ) return;

	} else if( CVarSystem::GetCVarInt( "hand" ) == 2 ) return;

	paliashdr =( dmdl_t * )currentmodel->extradata;

	if( currententity->flags &( RF_SHELL_HALF_DAM | RF_SHELL_GREEN | RF_SHELL_RED | RF_SHELL_BLUE | RF_SHELL_DOUBLE ) ) {

		shadelight = vec3_origin;
		if( currententity->flags & RF_SHELL_HALF_DAM ) {

				shadelight[ 0 ] = 0.56f;
				shadelight[ 1 ] = 0.59f;
				shadelight[ 2 ] = 0.45f;
		}
		if( currententity->flags & RF_SHELL_DOUBLE ) {

			shadelight[ 0 ] = 0.9f;
			shadelight[ 1 ] = 0.7f;
		}
		if( currententity->flags & RF_SHELL_RED ) shadelight[ 0 ] = 1.0f;
		if( currententity->flags & RF_SHELL_GREEN ) shadelight[ 1 ] = 1.0f;
		if( currententity->flags & RF_SHELL_BLUE ) shadelight[ 2 ] = 1.0f;
	} else if( currententity->flags & RF_FULLBRIGHT ) {

		for( int i = 0; i < 3; i++ ) shadelight[ i ] = 1.0f;
	} else {

		LightPoint( currententity->origin, &shadelight );
		
		float s = shadelight[ 0 ];

		if( s < shadelight[ 1 ] ) s = shadelight[ 1 ];
		if( s < shadelight[ 2 ] ) s = shadelight[ 2 ];

		shadelight[ 0 ] = s;
		shadelight[ 1 ] = s;
		shadelight[ 2 ] = s;
	}

	if( currententity->flags & RF_MINLIGHT ) {

		int i;
		for( i = 0; i < 3; i++ ) if( shadelight[ i ] > 0.1f ) break;
		if( i == 3 ) {

			shadelight[ 0 ] = 0.1f;
			shadelight[ 1 ] = 0.1f;
			shadelight[ 2 ] = 0.1f;
		}
	}

	if( currententity->flags & RF_GLOW ) {	// bonus items will pulse with time

		float	scale;
		float	min;

		scale = 0.1f * sin( r_newrefdef.time * 7.0f );
		for( int i = 0; i < 3; i++ ) {

			min = shadelight[ i ] * 0.8f;
			shadelight[ i ] += scale;
			if( shadelight[ i ] < min ) shadelight[ i ] = min;
		}
	}

// =================
// PGM	ir goggles color override
	if( r_newrefdef.rdflags & RDF_IRGOGGLES && currententity->flags & RF_IR_VISIBLE ) {

		shadelight[ 0 ] = 1.0f;
		shadelight[ 1 ] = 0.0f;
		shadelight[ 2 ] = 0.0f;
	}
// PGM	
// =================

	shadedots = r_avertexnormal_dots[( ( int )( currententity->angles[ 1 ] * ( SHADEDOT_QUANT / 360.0f ) ) ) &( SHADEDOT_QUANT - 1 ) ];
	
	an = currententity->angles[ 1 ] / 180.0f * M_PI;
	shadevector[ 0 ] = cos( -an );
	shadevector[ 1 ] = sin( -an );
	shadevector[ 2 ] = 1;
	shadevector.Normalize( );

	//
	// locate the proper data
	//

	c_alias_polys += paliashdr->num_tris;

	//
	// draw all the triangles
	//
	if( currententity->flags & RF_DEPTHHACK ) // hack the depth range to prevent view model from poking into walls
		glDepthRange( gldepthmin, gldepthmin + 0.3f * ( gldepthmax - gldepthmin ) );

	if( ( currententity->flags & RF_WEAPONMODEL ) &&( CVarSystem::GetCVarInt( "hand" ) == 1 ) ) {
		
		glMatrixMode( GL_PROJECTION );
		glPushMatrix( );
		glLoadIdentity( );
		glScalef( -1, 1, 1 );
	    SetProjection( r_newrefdef.fov_y, ( float )r_newrefdef.width /( float )r_newrefdef.height, 4.0f, 80000.0f );
		glMatrixMode( GL_MODELVIEW );
		glCullFace( GL_BACK );
	}

    glPushMatrix( );
	e->angles[ PITCH ] = -e->angles[ PITCH ];	// sigh.
	RotateForEntity( e );
	e->angles[ PITCH ] = -e->angles[ PITCH ];	// sigh.

	// select skin
	if( currententity->skin ) skin = currententity->skin;	// custom player skin
	else {

		if( currententity->skinnum >= MAX_MD2SKINS ) skin = currentmodel->skins[ 0 ];
		else {

			skin = currentmodel->skins[ currententity->skinnum ];
			if( !skin ) skin = currentmodel->skins[ 0 ];
		}
	}
	if( !skin ) skin = r_notexture;	// fallback...
	skin->Bind( );

	// draw it

	//glShadeModel( GL_SMOOTH );

	//GL_TexEnv( GL_MODULATE );
	if( currententity->flags & RF_TRANSLUCENT ) glEnable( GL_BLEND );

	if( ( currententity->frame >= paliashdr->num_frames )  ||( currententity->frame < 0 ) ) {

		Common::Com_Printf( "R_DrawAliasModel %s: no such frame %d\n", currentmodel->name.c_str( ), currententity->frame );
		currententity->frame = 0;
		currententity->oldframe = 0;
	}

	if( ( currententity->oldframe >= paliashdr->num_frames ) ||( currententity->oldframe < 0 ) ) {

		Common::Com_Printf( "R_DrawAliasModel %s: no such oldframe %d\n", currentmodel->name.c_str( ), currententity->oldframe );
		currententity->frame = 0;
		currententity->oldframe = 0;
	}

	if( !r_lerpmodels.GetFloat( ) ) currententity->backlerp = 0;
	GL_DrawAliasFrameLerp( paliashdr, currententity->backlerp );

	//GL_TexEnv( GL_REPLACE );
	//glShadeModel( GL_FLAT );

	glPopMatrix( );

	if( ( currententity->flags & RF_WEAPONMODEL ) && ( CVarSystem::GetCVarInt( "hand" ) == 1 ) ) {

		glMatrixMode( GL_PROJECTION );
		glPopMatrix( );
		glMatrixMode( GL_MODELVIEW );
		glCullFace( GL_FRONT );
	}

	if( currententity->flags & RF_TRANSLUCENT ) glDisable( GL_BLEND );

	if( currententity->flags & RF_DEPTHHACK ) glDepthRange( gldepthmin, gldepthmax );

	if( gl_shadows.GetBool( ) && !( currententity->flags &( RF_TRANSLUCENT | RF_WEAPONMODEL ) ) ) {

		glPushMatrix( );
		RotateForEntity( e );
		//glDisable( GL_TEXTURE_2D );
		glEnable( GL_BLEND );
		glColor4f( 0.0f, 0.0f, 0.5f, 0.5f );
		GL_DrawAliasShadow( paliashdr, currententity->frame );
		//glEnable( GL_TEXTURE_2D );
		glDisable( GL_BLEND );
		glPopMatrix( );
	}

	glColor4f( 1, 1, 1, 1 );
}


