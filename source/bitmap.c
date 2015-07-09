/* Gets bit number from bit mask */
int get_bit_num(int bitmask){
  int bit;

  if(bitmask == 0)
    return 0;

  while(bitmask > 0){
    if(bitmask&1 == 0)
      bit++;
    bitmask >>=1;
  }

  return bit++;
}

/* Gets bit mask from bit number */
int get_bit_mask(int bit){
  int mask = 1;

  return mask <<= bit;
}

int find_dev_mask(int dev){
  int mask = 1;
  int res = 0;

  while(mask < dev){
    if(mask & dev)
      res = mask;
    mask <<= 1;
  }

   return res;
}
