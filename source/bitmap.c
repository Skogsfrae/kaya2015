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
