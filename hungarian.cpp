/*
 * hungarian.cpp
 *
 *  Created on: Aug 30, 2014
 *      Author: yuxiang
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

void buildassignmentvector(float *assignment, bool *starMatrix, int nOfRows, int nOfColumns);
void computeassignmentcost(float *assignment, float *cost, float *distMatrix, int nOfRows, float infval);
void step2a(float *assignment, float *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim);
void step2b(float *assignment, float *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim);
void step3(float *assignment, float *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim);
void step4(float *assignment, float *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim, int row, int col);
void step5(float *assignment, float *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim);

void assignmentoptimal(float *assignment, float *cost, float *distMatrixIn, int nOfRows, int nOfColumns, float infval)
{
	float *distMatrix, *distMatrixTemp, *distMatrixEnd, *columnEnd, value, minValue;
	bool *coveredColumns, *coveredRows, *starMatrix, *newStarMatrix, *primeMatrix;
	int nOfElements, minDim, row, col;
	bool infiniteValueFound;
	float maxFiniteValue, infValue;

	/* initialization */
	*cost = 0;
	for(row = 0; row < nOfRows; row++)
		assignment[row] = -1.0;

	/* generate working copy of distance Matrix */
	/* check if all matrix elements are positive */
	nOfElements   = nOfRows * nOfColumns;
	distMatrix    = (float *)malloc(nOfElements * sizeof(float));
	distMatrixEnd = distMatrix + nOfElements;
	for(row = 0; row < nOfElements; row++)
	{
		value = distMatrixIn[row];
		if(value < infval && value < 0)
			printf("All matrix elements have to be non-negative.\n");
		distMatrix[row] = value;
	}

	/* check for infinite values */
	maxFiniteValue     = -1;
	infiniteValueFound = false;
	distMatrixTemp = distMatrix;
	while(distMatrixTemp < distMatrixEnd)
	{
		value = *distMatrixTemp++;
		if(value < infval)
		{
			if(value > maxFiniteValue)
				maxFiniteValue = value;
		}
		else
			infiniteValueFound = true;
	}
	if(infiniteValueFound)
	{
		if(maxFiniteValue == -1) /* all elements are infinite */
			return;

		/* set all infinite elements to big finite value */
		if(maxFiniteValue > 0)
			infValue = 10 * maxFiniteValue * nOfElements;
		else
			infValue = 10;
		distMatrixTemp = distMatrix;
		while(distMatrixTemp < distMatrixEnd)
		{
			if(*distMatrixTemp++ >= infval)
				*(distMatrixTemp-1) = infValue;
		}
	}

	/* memory allocation */
	coveredColumns = (bool *)malloc(nOfColumns * sizeof(bool));
	coveredRows    = (bool *)malloc(nOfRows * sizeof(bool));
	starMatrix     = (bool *)malloc(nOfElements * sizeof(bool));
	primeMatrix    = (bool *)malloc(nOfElements * sizeof(bool));
	newStarMatrix  = (bool *)malloc(nOfElements * sizeof(bool)); /* used in step4 */

	memset(coveredColumns, 0, sizeof(bool) * nOfColumns);
	memset(coveredRows, 0, sizeof(bool) * nOfRows);
	memset(starMatrix, 0, sizeof(bool) * nOfElements);
	memset(primeMatrix, 0, sizeof(bool) * nOfElements);
	memset(newStarMatrix, 0, sizeof(bool) * nOfElements);
	/* preliminary steps */
	if(nOfRows <= nOfColumns)
	{
		minDim = nOfRows;
		for(row = 0; row < nOfRows; row++)
		{
			/* find the smallest element in the row */
			distMatrixTemp = distMatrix + row;
			minValue = *distMatrixTemp;
			distMatrixTemp += nOfRows;
			while(distMatrixTemp < distMatrixEnd)
			{
				value = *distMatrixTemp;
				if(value < minValue)
					minValue = value;
				distMatrixTemp += nOfRows;
			}

			/* subtract the smallest element from each element of the row */
			distMatrixTemp = distMatrix + row;
			while(distMatrixTemp < distMatrixEnd)
			{
				*distMatrixTemp -= minValue;
				distMatrixTemp += nOfRows;
			}
		}

		/* Steps 1 and 2a */
		for(row = 0; row < nOfRows; row++)
		{
			for(col = 0; col < nOfColumns; col++)
			{
				if(distMatrix[row + nOfRows*col] == 0)
				{
					if(!coveredColumns[col])
					{
						starMatrix[row + nOfRows*col] = true;
						coveredColumns[col]           = true;
						break;
					}
				}
			}
		}
	}
	else /* if(nOfRows > nOfColumns) */
	{
		minDim = nOfColumns;
		for(col = 0; col < nOfColumns; col++)
		{
			/* find the smallest element in the column */
			distMatrixTemp = distMatrix     + nOfRows*col;
			columnEnd      = distMatrixTemp + nOfRows;

			minValue = *distMatrixTemp++;
			while(distMatrixTemp < columnEnd)
			{
				value = *distMatrixTemp++;
				if(value < minValue)
					minValue = value;
			}

			/* subtract the smallest element from each element of the column */
			distMatrixTemp = distMatrix + nOfRows*col;
			while(distMatrixTemp < columnEnd)
				*distMatrixTemp++ -= minValue;
		}

		/* Steps 1 and 2a */
		for(col = 0; col < nOfColumns; col++)
		{
			for(row = 0; row < nOfRows; row++)
			{
				if(distMatrix[row + nOfRows*col] == 0)
				{
					if(!coveredRows[row])
					{
						starMatrix[row + nOfRows*col] = true;
						coveredColumns[col]           = true;
						coveredRows[row]              = true;
						break;
					}
				}
			}
		}
		for(row = 0; row < nOfRows; row++)
			coveredRows[row] = false;
	}

	/* move to step 2b */
	step2b(assignment, distMatrix, starMatrix, newStarMatrix, primeMatrix, coveredColumns, coveredRows, nOfRows, nOfColumns, minDim);
	/* compute cost and remove invalid assignments */
	computeassignmentcost(assignment, cost, distMatrixIn, nOfRows, infval);

	// printBoolMatrix(starMatrix, nOfRows, nOfColumns, "final assignment");
	/* free allocated memory */
	free(distMatrix);
	free(coveredColumns);
	free(coveredRows);
	free(starMatrix);
	free(primeMatrix);
	free(newStarMatrix);

	return;
}


void buildassignmentvector(float *assignment, bool *starMatrix, int nOfRows, int nOfColumns)
{
	int row, col;

	for(row = 0; row < nOfRows; row++)
	{
		for(col = 0; col < nOfColumns; col++)
		{
			if(starMatrix[row + nOfRows*col])
			{
				assignment[row] = col;
				break;
			}
		}
	}
}


void computeassignmentcost(float *assignment, float *cost, float *distMatrix, int nOfRows, float infval)
{
	int row, col;
	float value;

	for(row = 0; row < nOfRows; row++)
	{
		col = assignment[row];
		if(col >= 0)
		{
			value = distMatrix[row + nOfRows*col];
			if(value < infval)
				*cost += value;
			else
				assignment[row] = -1.0;
		}
	}
}


void step2a(float *assignment, float *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim)
{
	bool *starMatrixTemp, *columnEnd;
	int col;

	/* cover every column containing a starred zero */
	for(col=0; col<nOfColumns; col++)
	{
		starMatrixTemp = starMatrix     + nOfRows*col;
		columnEnd      = starMatrixTemp + nOfRows;
		while(starMatrixTemp < columnEnd){
			if(*starMatrixTemp++)
			{
				coveredColumns[col] = true;
				break;
			}
		}
	}

	/* move to step 3 */
	step2b(assignment, distMatrix, starMatrix, newStarMatrix, primeMatrix, coveredColumns, coveredRows, nOfRows, nOfColumns, minDim);
}


void step2b(float *assignment, float *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim)
{
	int col, nOfCoveredColumns;

	/* count covered columns */
	nOfCoveredColumns = 0;
	for(col=0; col<nOfColumns; col++)
		if(coveredColumns[col])
			nOfCoveredColumns++;

	if(nOfCoveredColumns == minDim)
	{
		/* algorithm finished */
		buildassignmentvector(assignment, starMatrix, nOfRows, nOfColumns);
	}
	else
	{
		/* move to step 3 */
		step3(assignment, distMatrix, starMatrix, newStarMatrix, primeMatrix, coveredColumns, coveredRows, nOfRows, nOfColumns, minDim);
	}

}


void step3(float *assignment, float *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim)
{
	bool zerosFound;
	int row, col, starCol;

	zerosFound = true;
	while(zerosFound)
	{
		zerosFound = false;
		for(col = 0; col < nOfColumns; col++)
		{
			if(!coveredColumns[col])
			{
				for(row = 0; row < nOfRows; row++)
				{
					if((!coveredRows[row]) && (distMatrix[row + nOfRows*col] == 0))
					{
						/* prime zero */
						primeMatrix[row + nOfRows*col] = true;

						/* find starred zero in current row */
						for(starCol = 0; starCol < nOfColumns; starCol++)
							if(starMatrix[row + nOfRows*starCol])
								break;

						if(starCol == nOfColumns) /* no starred zero found */
						{
							/* move to step 4 */
							step4(assignment, distMatrix, starMatrix, newStarMatrix, primeMatrix, coveredColumns, coveredRows, nOfRows, nOfColumns, minDim, row, col);
							return;
						}
						else
						{
							coveredRows[row]        = true;
							coveredColumns[starCol] = false;
							zerosFound              = true;
							break;
						}
					}
				}
			}
		}
	}

	/* move to step 5 */
	step5(assignment, distMatrix, starMatrix, newStarMatrix, primeMatrix, coveredColumns, coveredRows, nOfRows, nOfColumns, minDim);
}


void step4(float *assignment, float *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim, int row, int col)
{
	int n, starRow, starCol, primeRow, primeCol;
	int nOfElements = nOfRows*nOfColumns;

	/* generate temporary copy of starMatrix */
	for(n = 0; n < nOfElements; n++)
		newStarMatrix[n] = starMatrix[n];

	/* star current zero */
	newStarMatrix[row + nOfRows*col] = true;

	/* find starred zero in current column */
	starCol = col;
	for(starRow = 0; starRow < nOfRows; starRow++)
	{
		if(starMatrix[starRow + nOfRows*starCol])
			break;
	}

	while(starRow < nOfRows)
	{
		/* unstar the starred zero */
		newStarMatrix[starRow + nOfRows*starCol] = false;

		/* find primed zero in current row */
		primeRow = starRow;
		for(primeCol = 0; primeCol < nOfColumns; primeCol++)
		{
			if(primeMatrix[primeRow + nOfRows*primeCol])
				break;
		}

		/* star the primed zero */
		newStarMatrix[primeRow + nOfRows*primeCol] = true;

		/* find starred zero in current column */
		starCol = primeCol;
		for(starRow = 0; starRow < nOfRows; starRow++)
		{
			if(starMatrix[starRow + nOfRows*starCol])
				break;
		}
	}

	/* use temporary copy as new starMatrix */
	/* delete all primes, uncover all rows */
	for(n = 0; n < nOfElements; n++)
	{
		primeMatrix[n] = false;
		starMatrix[n]  = newStarMatrix[n];
	}
	for(n = 0; n < nOfRows; n++)
		coveredRows[n] = false;

	/* move to step 2a */
	step2a(assignment, distMatrix, starMatrix, newStarMatrix, primeMatrix, coveredColumns, coveredRows, nOfRows, nOfColumns, minDim);
}


void step5(float *assignment, float *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim)
{
	float h, value;
	int row, col;

	/* find smallest uncovered element h */
	h = PLUS_INFINITY;
	for(row = 0; row < nOfRows; row++)
	{
		if(!coveredRows[row])
		{
			for(col = 0; col < nOfColumns; col++)
			{
				if(!coveredColumns[col])
				{
					value = distMatrix[row + nOfRows*col];
					if(value < h)
						h = value;
				}
			}
		}
	}

	/* add h to each covered row */
	for(row = 0; row < nOfRows; row++)
	{
		if(coveredRows[row])
		{
			for(col = 0; col < nOfColumns; col++)
				distMatrix[row + nOfRows*col] += h;
		}
	}

	/* subtract h from each uncovered column */
	for(col = 0; col < nOfColumns; col++)
	{
		if(!coveredColumns[col])
		{
			for(row = 0; row < nOfRows; row++)
				distMatrix[row + nOfRows*col] -= h;
		}
	}

	/* move to step 3 */
	step3(assignment, distMatrix, starMatrix, newStarMatrix, primeMatrix, coveredColumns, coveredRows, nOfRows, nOfColumns, minDim);
}
