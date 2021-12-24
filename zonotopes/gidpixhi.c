#include <stdio.h>

// Sets to Minkowski sum:
//   Cycles (4) and sign changes (2) of (0, 0, 0, 1):
//     (0, 0, 0, 1), (0, 0, 0, 2) x8
//   Even (first 3: 4) and all (last: 2) sign changes (8) of (1, 1, 1, 1):
//     (1, 1, 1, 1), (2, 2, 2, 2) x16
//   Even perms (12) and even sign changes (4) of (0, phi-1, 1, phi):
//     (0, 0, 1, 1), (0, 1, 1, 2), (0, 1, 2, 3) x48
//   Given by v1, v2, and v3, respectively
// Max coord 46 - leave 1 margin for bucket fill, g is 95^4

// i = vector index
// j = position along vector
// k = coordinate index
// s = segment length to apply perms/sign changes
// g indexed by x, y, z, w
// gFlat indexed by n
// v indexed by i, j, k
// vOffsets indexed by i, j
// u and verts indexed by i, k, (unnamed)
// Golden ring is a+b*phi, pointed to by p, q

int v1[8][2][4] =
  {{{00, 00, 00, +1}, {00, 00, 00, +2}},
   {{00, 00, 00, -1}, {00, 00, 00, -2}},
   {{00, 00, +1, 00}, {00, 00, +2, 00}},
   {{00, 00, -1, 00}, {00, 00, -2, 00}},
   {{00, +1, 00, 00}, {00, +2, 00, 00}},
   {{00, -1, 00, 00}, {00, -2, 00, 00}},
   {{+1, 00, 00, 00}, {+2, 00, 00, 00}},
   {{-1, 00, 00, 00}, {-2, 00, 00, 00}}};

int v1Offsets[8*2];

int v2[8][2][4] =
  {{{+1, +1, +1, +1}, {+2, +2, +2, +2}},
   {{+1, +1, +1, -1}, {+2, +2, +2, -2}},
   {{+1, -1, -1, +1}, {+2, -2, -2, +2}},
   {{+1, -1, -1, -1}, {+2, -2, -2, -2}},
   {{-1, +1, -1, +1}, {-2, +2, -2, +2}},
   {{-1, +1, -1, -1}, {-2, +2, -2, -2}},
   {{-1, -1, +1, +1}, {-2, -2, +2, +2}},
   {{-1, -1, +1, -1}, {-2, -2, +2, -2}}};

int v2Offsets[8*2];

// Remaining rows filled in main()
int v3[48][3][4] =
  {{{00, 00, +1, +1}, {00, +1, +1, +2}, {00, +1, +2, +3}},
   {{00, 00, -1, -1}, {00, +1, -1, -2}, {00, +1, -2, -3}},
   {{00, 00, +1, -1}, {00, -1, +1, -2}, {00, -1, +2, -3}},
   {{00, 00, -1, +1}, {00, -1, -1, +2}, {00, -1, -2, +3}}};

int v3Offsets[48*3];

int u1[8][4][2] =
  {{{00, 00}, {00, 00}, {00, 00}, {+1, 00}},
   {{00, 00}, {00, 00}, {00, 00}, {-1, 00}},
   {{00, 00}, {00, 00}, {+1, 00}, {00, 00}},
   {{00, 00}, {00, 00}, {-1, 00}, {00, 00}},
   {{00, 00}, {+1, 00}, {00, 00}, {00, 00}},
   {{00, 00}, {-1, 00}, {00, 00}, {00, 00}},
   {{+1, 00}, {00, 00}, {00, 00}, {00, 00}},
   {{-1, 00}, {00, 00}, {00, 00}, {00, 00}}};

int u2[8][4][2] =
  {{{+1, 00}, {+1, 00}, {+1, 00}, {+1, 00}},
   {{+1, 00}, {+1, 00}, {+1, 00}, {-1, 00}},
   {{+1, 00}, {-1, 00}, {-1, 00}, {+1, 00}},
   {{+1, 00}, {-1, 00}, {-1, 00}, {-1, 00}},
   {{-1, 00}, {+1, 00}, {-1, 00}, {+1, 00}},
   {{-1, 00}, {+1, 00}, {-1, 00}, {-1, 00}},
   {{-1, 00}, {-1, 00}, {+1, 00}, {+1, 00}},
   {{-1, 00}, {-1, 00}, {+1, 00}, {-1, 00}}};

int u3[48][4][2] =
  {{{00, 00}, {-1, +1}, {+1, 00}, {00, +1}},
   {{00, 00}, {-1, +1}, {-1, 00}, {00, -1}},
   {{00, 00}, {+1, -1}, {+1, 00}, {00, -1}},
   {{00, 00}, {+1, -1}, {-1, 00}, {00, +1}}};

int evenPerms[12][4] =
  {{0, 1, 2, 3}, {0, 2, 3, 1}, {0, 3, 1, 2},
   {1, 0, 3, 2}, {1, 2, 0, 3}, {1, 3, 2, 0},
   {2, 0, 1, 3}, {2, 1, 3, 0}, {2, 3, 0, 1},
   {3, 0, 2, 1}, {3, 1, 0, 2}, {3, 2, 1, 0}};

int cyclicPerms[4][4] =
  {{0, 1, 2, 3}, {1, 2, 3, 0}, {2, 3, 0, 1}, {3, 0, 1, 2}};

int choosePerms[6][4] =
  {{0, 1, 2, 3}, {1, 2, 3, 0}, {2, 3, 0, 1},
   {3, 0, 1, 2}, {0, 2, 1, 3}, {3, 1, 2, 0}};

int bBox[2][4];

const int N = 95*95*95*95;

char g[95][95][95][95];
char *gFlat = (char*)g;

#define numHiSections 7
int hiSections[numHiSections] =
  {24, 64, 64, 64, 96, 96, 192};
#define numHiVerts 600
int hiVerts[numHiVerts][4][2] =
  // All permutations of:
  {{{2, 2}, {2, 2}, {0, 0}, {0, 0}},  // 6*4=24
   {{1, 3}, {1, 1}, {1, 1}, {1, 1}},  // 4*16=64
   {{1, 2}, {1, 2}, {1, 2}, {1, 0}},  // 4*16=64
   {{2, 3}, {0, 1}, {0, 1}, {0, 1}},  // 4*16=64
   // All even permutations of:
   //{{2, 3}, {1, 1}, {1, 0}, {0, 0}},  // 12*8=96
   //{{1, 2}, {1, 3}, {0, 0}, {0, 1}},  // 12*8=96
   //{{1, 2}, {1, 1}, {2, 2}, {0, 1}}}; // 12*16=192
   {{1, 1}, {2, 3}, {1, 0}, {0, 0}},  // 12*8=96
   {{1, 3}, {1, 2}, {0, 0}, {0, 1}},  // 12*8=96
   {{1, 1}, {1, 2}, {2, 2}, {0, 1}}}; // 12*16=192

#define numRahiSections 10
int rahiSections[numRahiSections] =
  {48, 32, 64, 96, 96, 96, 192, 192, 192, 192};
#define numRahiVerts 1200
int rahiVerts[numRahiVerts][4][2] =
  // All permutations of:
  {{{0, 0}, {0, 0}, {0, 2}, {2, 4}},  // 12*4=48
   {{0, 0}, {2, 2}, {2, 2}, {2, 2}},  // 4*8=32
   {{1, 1}, {1, 1}, {1, 1}, {3, 3}},  // 4*16=64
   {{1, 1}, {1, 1}, {1, 3}, {1, 3}},  // 6*16=96
   // All even permutations of:
   {{0, 0}, {1, 0}, {2, 3}, {1, 3}},  // 12*8=96
   {{0, 0}, {0, 1}, {3, 3}, {1, 2}},  // 12*8=96
   {{1, 0}, {0, 1}, {2, 4}, {1, 1}},  // 12*16=192
   {{1, 0}, {1, 1}, {2, 3}, {2, 2}},  // 12*16=192
   {{0, 1}, {1, 2}, {1, 3}, {2, 2}},  // 12*16=192
   {{1, 1}, {0, 2}, {2, 3}, {1, 2}}}; // 12*16=192

#define numRoxSections 6
int roxSections[numRoxSections] =
  {48, 96, 96, 96, 192, 192};
#define numRoxVerts 720
int roxVerts[numRoxVerts][4][2] =
  // All permutations of:
  {{{0, 0}, {0, 0}, {0, 2}, {2, 2}},  // 12*4=48
   {{1, 0}, {1, 0}, {1, 2}, {1, 2}},  // 6*16=96
   // All even permutations of:
   {{0, 0}, {1, 0}, {0, 1}, {1, 3}},  // 12*8=96
   {{0, 0}, {1, 1}, {1, 2}, {2, 1}},  // 12*8=96
   {{1, 0}, {0, 1}, {2, 2}, {1, 1}},  // 12*16=192
   {{0, 1}, {1, 1}, {0, 2}, {1, 2}}}; // 12*16=192

#define numExSections 3
int exSections[numExSections] =
  {8, 16, 96};
#define numExVerts 120
int exVerts[numExVerts][4][2] = 
  // All permutations of:
  {{{0, 2}, {0, 0}, {0, 0}, {0, 0}},  // 4*2=8
   {{0, 1}, {0, 1}, {0, 1}, {0, 1}},  // 1*16=16
   // All even permutations of:
   {{1, 1}, {0, 1}, {1, 0}, {0, 0}}}; // 12*8=96

void minkowski(int* offset, int jMax, char place){
  char tmpPlace = place*2;
  for (int x = bBox[0][0]; x < bBox[1][0]; x++) {
    for (int y = bBox[0][1]; y < bBox[1][1]; y++) {
      for (int z = bBox[0][2]; z < bBox[1][2]; z++) {
	for (int w = bBox[0][3]; w < bBox[1][3]; w++) {
	  int n = ((x*95+y)*95+z)*95+w;
	  if (gFlat[n]&place) {
	    for (int j = 0; j < jMax; j++) {
	      gFlat[n+offset[j]] |= tmpPlace;
	    }
	  }
	}
      }
    }
  }
  int offsetFromMid = offset[jMax-1]+N/2;
  int offsetCoords[4] =
    {(offsetFromMid/(95*95*95))-47,
     ((offsetFromMid/(95*95))%95)-47,
     ((offsetFromMid/95)%95)-47,
     (offsetFromMid%95)-47};
  for (int k = 0; k < 4; k++) {
    if (offsetCoords[k] < 0) {
      bBox[0][k] += offsetCoords[k];
      if (bBox[0][k] < 0) {
	bBox[0][k] = 0;
      }
    } else {
      bBox[1][k] += offsetCoords[k];
      if (bBox[1][k] >= 95) {
	bBox[1][k] = 95;
      }
    }
  }
  for (int x = bBox[0][0]; x < bBox[1][0]; x++) {
    for (int y = bBox[0][1]; y < bBox[1][1]; y++) {
      for (int z = bBox[0][2]; z < bBox[1][2]; z++) {
	for (int w = bBox[0][3]; w < bBox[1][3]; w++) {
	  int n = ((x*95+y)*95+z)*95+w;
	  if (gFlat[n]&tmpPlace) {
	    gFlat[n] &= ~tmpPlace;
	    gFlat[n] |= place;
	  }
	}
      }
    }
  }
}

void minkowskiAll(int* offsets, int iMax, int jMax, char place) {
  for (int i = 0; i < iMax; i++) {
    minkowski(offsets+i*jMax, jMax, place);
    printf("Done with %d\n", i);
  }
}

/*
void checkErrors() {
  char line[93];
  FILE *binFile = fopen("../python/gidpixhi/bin.txt", "r");
  for (int x = 0; x < 93; x++) {
    for (int y = 0; y < 93; y++) {
      for (int z = 0; z < 93; z++) {
	int foo = fscanf(binFile, "%s", line);
	int w1 = 0;
	while (w1 < 93 && line[w1] == '0') w1++;
	int w2 = 0;
	while (w2 < 93 && g[x+1][y+1][z+1][w2+1] == 0) w2++;
	if (w1 != w2) {
	  printf("%d %d %d %d %d\n", x, y, z, w1, w2);
	}
      }
    }
  }
  fclose(binFile);
}
*/

void init3s() {
  for (int i = 4; i < 48; i++) {
    for (int j = 0; j < 3; j++) {
      for (int k = 0; k < 4; k++) {
	v3[i][j][k] = v3[i%4][j][evenPerms[i/4][k]];
      }
    }
  }
  for (int i = 4; i < 48; i++) {
    for (int k = 0; k < 4; k++) {
      u3[i][k][0] = u3[i%4][evenPerms[i/4][k]][0];
      u3[i][k][1] = u3[i%4][evenPerms[i/4][k]][1];
    }
  }
}

int getOffset(int *vRow) {
  return ((vRow[0]*95+vRow[1])*95+vRow[2])*95+vRow[3];
}

void initOffsets() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 2; j++) {
      v1Offsets[i*2+j] = getOffset(v1[i][j]);
    }
  }
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 2; j++) {
      v2Offsets[i*2+j] = getOffset(v2[i][j]);
    }
  }
  for (int i = 0; i < 48; i++) {
    for (int j = 0; j < 3; j++) {
      v3Offsets[i*3+j] = getOffset(v3[i][j]);
    }
  }
}

int goldenSign(int a, int b) {
  int c = a*a+a*b-b*b;
  if (c < 0) {
    return (b > 0) ? 1 : -1;
  }
  if (c > 0) {
    return (a > 0) ? 1 : -1;
  }
  return 0;
}

int dotProdSign(int p[4][2], int q[4][2]) {
  int a = 0;
  int b = 0;
  for (int k = 0; k < 4; k++) {
    a += p[k][0]*q[k][0]+p[k][1]*q[k][1];
    b += p[k][0]*q[k][1]+p[k][1]*q[k][0]+p[k][1]*q[k][1];
  }
  return goldenSign(a, b);
}

void fillSignChanges(int p[][4][2], int n, int s, int kChange) {
  for (int i = s; i < 2*s; i++) {
    for (int k = 0; k < 4; k++) {
      p[n+i][k][0] = p[n+i-s][k][0];
      p[n+i][k][1] = p[n+i-s][k][1];
    }
    p[n+i][kChange][0] *= -1;
    p[n+i][kChange][1] *= -1;
  }
}

void fillPerms(int p[][4][2], int n, int s, int perms[][4], int numPerms) {
  for (int i = s; i < numPerms*s; i++) {
    for (int k = 0; k < 4; k++) {
      int permIdx = perms[i/s][k];
      p[n+i][k][0] = p[n+i%s][permIdx][0];
      p[n+i][k][1] = p[n+i%s][permIdx][1];
    }
  }
}

void fillAllSignChanges(int starts[], int numSections, int verts[][4][2], int sections[]) {
  int start = 0;
  for (int section = 0; section < numSections; section++) {
    for (int k = 0; k < 4; k++) {
      verts[start][k][0] = verts[section][k][0];
      verts[start][k][1] = verts[section][k][1];
    }
    starts[section] = start;
    start += sections[section];
  }
  for (int section = 0; section < numSections; section++) {
    start = starts[section];
    int s = 1;
    for (int k = 0; k < 4; k++) {
      if (verts[start][k][0] != 0 || verts[start][k][1] != 0) {
	fillSignChanges(verts, start, s, k);
	s *= 2;
      }
    }
  }
}

void initVerts() {
  int hiStarts[numHiSections];
  fillAllSignChanges(hiStarts, numHiSections, hiVerts, hiSections);
  fillPerms(hiVerts, hiStarts[0], 4, choosePerms, 6);
  fillPerms(hiVerts, hiStarts[1], 16, cyclicPerms, 4);
  fillPerms(hiVerts, hiStarts[2], 16, cyclicPerms, 4);
  fillPerms(hiVerts, hiStarts[3], 16, cyclicPerms, 4);
  fillPerms(hiVerts, hiStarts[4], 8, evenPerms, 12);
  fillPerms(hiVerts, hiStarts[5], 8, evenPerms, 12);
  fillPerms(hiVerts, hiStarts[6], 16, evenPerms, 12);
  int rahiStarts[numRahiSections];
  fillAllSignChanges(rahiStarts, numRahiSections, rahiVerts, rahiSections);
  fillPerms(rahiVerts, rahiStarts[0], 4, evenPerms, 12);
  fillPerms(rahiVerts, rahiStarts[1], 8, cyclicPerms, 4);
  fillPerms(rahiVerts, rahiStarts[2], 16, cyclicPerms, 4);
  fillPerms(rahiVerts, rahiStarts[3], 16, choosePerms, 6);
  fillPerms(rahiVerts, rahiStarts[4], 8, evenPerms, 12);
  fillPerms(rahiVerts, rahiStarts[5], 8, evenPerms, 12);
  fillPerms(rahiVerts, rahiStarts[6], 16, evenPerms, 12);
  fillPerms(rahiVerts, rahiStarts[7], 16, evenPerms, 12);
  fillPerms(rahiVerts, rahiStarts[8], 16, evenPerms, 12);
  fillPerms(rahiVerts, rahiStarts[9], 16, evenPerms, 12);
  int roxStarts[numRoxSections];
  fillAllSignChanges(roxStarts, numRoxSections, roxVerts, roxSections);
  fillPerms(roxVerts, roxStarts[0], 4, evenPerms, 12);
  fillPerms(roxVerts, roxStarts[1], 16, choosePerms, 6);
  fillPerms(roxVerts, roxStarts[2], 8, evenPerms, 12);
  fillPerms(roxVerts, roxStarts[3], 8, evenPerms, 12);
  fillPerms(roxVerts, roxStarts[4], 16, evenPerms, 12);
  fillPerms(roxVerts, roxStarts[5], 16, evenPerms, 12);
  int exStarts[numExSections];
  fillAllSignChanges(exStarts, numExSections, exVerts, exSections);
  fillPerms(exVerts, exStarts[0], 2, cyclicPerms, 4);
  fillPerms(exVerts, exStarts[2], 8, evenPerms, 12);
}

void initMinkowski(int offset, char place) {
  gFlat[offset] |= place;
  int coords[4] =
    {offset/(95*95*95),
     (offset/(95*95))%95,
     (offset/95)%95,
     offset%95};
  for (int k = 0; k < 4; k++) {
    bBox[0][k] = coords[k];
    bBox[1][k] = coords[k]+1;
  }
}

#define cellBuf 64
void makeCell(char place, int verts[][4][2], int vert) {
  int segsAdded = 0;
  int cellOffset = N/2;
  for (int i = 0; i < 8; i++) {
    if (dotProdSign(verts[vert], u1[i]) == 1) {
      cellOffset += v1Offsets[2*i+1];
    }
  }
  for (int i = 0; i < 8; i++) {
    if (dotProdSign(verts[vert], u2[i]) == 1) {
      cellOffset += v2Offsets[2*i+1];
    }
  }
  for (int i = 0; i < 48; i++) {
    if (dotProdSign(verts[vert], u3[i]) == 1) {
      cellOffset += v3Offsets[3*i+2];
    }
  }
  initMinkowski(cellOffset, cellBuf);
  for (int i = 0; i < 8; i++) {
    if (dotProdSign(verts[vert], u1[i]) == 0) {
      minkowski(v1Offsets+2*i, 2, cellBuf);
      segsAdded++;
    }
  }
  for (int i = 0; i < 8; i++) {
    if (dotProdSign(verts[vert], u2[i]) == 0) {
      minkowski(v2Offsets+2*i, 2, cellBuf);
      segsAdded += 2;
    }
  }
  for (int i = 0; i < 48; i++) {
    if (dotProdSign(verts[vert], u3[i]) == 0) {
      minkowski(v3Offsets+3*i, 3, cellBuf);
      segsAdded += 2;
    }
  }
  for (int x = bBox[0][0]; x < bBox[1][0]; x++) {
    for (int y = bBox[0][1]; y < bBox[1][1]; y++) {
      for (int z = bBox[0][2]; z < bBox[1][2]; z++) {
	for (int w = bBox[0][3]; w < bBox[1][3]; w++) {
	  int n = ((x*95+y)*95+z)*95+w;
	  if (gFlat[n]&cellBuf) {
	    gFlat[n] &= ~cellBuf;
	    gFlat[n] |= place;
	  }
	}
      }
    }
  }
  printf("Done with %d: added %d segs\n", vert, segsAdded/2);
}

void makeCells(char place, int numVerts, int verts[][4][2]) {
  for (int vert = 0; vert < numVerts; vert++) {
    makeCell(place, verts, vert);
  }
}

void writeFile() {
  FILE *fp = fopen("voxels.txt", "w");
  for (int n = 0; n < N; n++) {
    fputc(gFlat[n]+32, fp);
    if (n%95 == 0) {
      fputc('\n', fp);
    }
  }
  fclose(fp);
}

int main() {
  init3s();
  initOffsets();
  initVerts();
  for (int n = 0; n < N; n++) {
    gFlat[n] = 0;
  }
  initMinkowski(N/2, 1);
  printf("Starting v1\n");
  minkowskiAll((int*)v1Offsets, 8, 2, 1);
  printf("Starting v2\n");
  minkowskiAll((int*)v2Offsets, 8, 2, 1);
  printf("Starting v3\n");
  minkowskiAll((int*)v3Offsets, 48, 3, 1);
  printf("Starting hi verts\n");
  makeCells(2, numHiVerts, hiVerts);
  printf("Starting rahi verts\n");
  makeCells(4, numRahiVerts, rahiVerts);
  printf("Starting rox verts\n");
  makeCells(8, numRoxVerts, roxVerts);
  printf("Starting ex verts\n");
  makeCells(16, numExVerts, exVerts);
  //checkErrors(); 
  writeFile();
}
