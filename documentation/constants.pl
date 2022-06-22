#!/usr/bin/perl

open FILE, "protocol.txt";
  while ($line = <FILE>) {
    if ($line =~ /^PACKET_/) {
      ($type, $const) = $line =~ /(PACKET_[0-9A-Z\_]+)\s+(\S+)/;
      $types{$type} = $const;
      $t = $types{$type};
      if ($t < $negative) {$negative = $t; };
      if ($t > $positive) {$positive = $t; };
    };
  };
close FILE;

#for ($i = -4096; $i < 240; $i++) {
#  $strings[4096 + $i] = 'PACKET_UNDEFINED';
#};

@types = sort {($types{$a} >= 0) <=> ($types{$b} >= 0) or abs($types{$a}) <=> abs($types{$b})} keys %types;
foreach $type (@types) {
  print "#define $type $types{$type}\n";
  $strings[$types{$type} - $negative] = $type;
};

print "#define PACKET_TYPE_LIMIT " . ($positive + 1) . "\n";

$" = '", "';
print "const char *packet_type_string[] = {\"@strings\"};\n";

for ($i = 0; $i < @types; $i++) {
  next if $types[$i] eq 'PACKET_KEEPALIVE';
  print "  $types[$i],\n";
};
print "  PACKET_KEEPALIVE\n";

print "%type = (\n";
foreach $type (@types) {
  print "  '$type' => $types{$type},\n";
};
print ");\n";

print "%string = (\n";
foreach $type (@types) {
  print "  $types{$type} => '$type',\n";
};
print ");\n";
