module test_design (n1, n2, n3, n4, n5);
  input n1, n2, n3;
  output n4, n5;
  wire n6, n7;

  and g1 (n6, n1, n2);
  or g2 (n7, n6, n3);
  not g3 (n4, n7);
  xor g4 (n5, n4, n1);
endmodule
