#! /usr/local/bin/perl
# $Id: mle2raw.pl,v 1.1.1.1 1998/10/28 21:07:35 alex Exp $
# mle2raw.pl router_number < src.mle > data.raw

sub outw {
  my ($par) = @_;

  print pack("n", $par);
#  print $par."\n";
}

sub outb {
  my ($par) = @_;

  print pack("C", $par);
#  print $par."\n";
}

sub printlinkbound {
  my ($link) = @_;

  for $intervalindex (0..7) {
    outw($intervals{$router."-".$link."-".$intervalindex});
  }
}

sub printlinkoutput {
  my ($link) = @_;

  for $intervalindex (sort {$b cmp $a} 0..7) {
    outb(2**$lists{$router."-".$link."-".$intervalindex."-0"});
    outb(0);
  }
}

die "usage: mle2raw.pl router_number < src.mle > data.raw" if ($#ARGV != 0);

print STDERR "This router got instance number : $ARGV[0]\n";
$router = $ARGV[0];
shift @ARGV;

while ($line = <>) {
  ($line =~ m/SET\s+routeur_instance\[(.+)\]\s+\(\s+intervals\[(.+)\]\[(.+)\]\s+:=\s+(.+)\s\)/) && do {
#   print $line;
#   print "intervals[".$1."-".$2."-".$3."] = ".$4."\n";
    $intervals{$1."-".$2."-".$3} = $4;
  };
 
  ($line =~ m/SET\s+routeur_instance\[(.+)\]\s+\(\s+lists\[(.+)\]\[(.+)\]\[(.+)\]\s+:=\s+(.+)\s\)/) && do {
#   print $line;
#   print "lists[".$1."-".$2."-".$3."-".$4."] = ".$5."\n";
    $lists{$1."-".$2."-".$3."-".$4} = $5;
  };
}

printlinkbound(0); printlinkbound(2); printlinkbound(4); printlinkbound(6);
printlinkoutput(6); printlinkoutput(4); printlinkoutput(2); printlinkoutput(0);
outb(0);
outb(255);
printlinkbound(1); printlinkbound(3); printlinkbound(5); printlinkbound(7);
printlinkoutput(7); printlinkoutput(5); printlinkoutput(3); printlinkoutput(1);

__END__
