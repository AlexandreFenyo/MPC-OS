#! /usr/bin/perl
#! /usr/local/bin/perl
#! /usr/local/bin/perl5
# $Id: transcript.pl,v 1.1.1.1 1998/10/28 21:07:31 alex Exp $

# @node:fct(param) <=> @node,reply,<T>:fct(param)

# load input
while (<>) {
    $_ =~ s%//.*@.*$%%;
    $input .= $_;
}

$varindex = -1;

# loop while there is something to do
while ($input =~ m/(^.*)@([^:]*):([^(]*)\(/m) {

  $varindex++;

  ($prev, $pair, $fct) = ($1, $2, $3);
  print STDERR "parsing ".$prev."@".$pair.":".$fct."(...)\n";

# any parameter ? 
  $input =~ m/^.*@[^:]*:[^(]*\((.*)/m;
  $firstcars = $1;

# any args ?
  $args = "";
  ($pair =~ s/,(.*)//) && ($args = $1);
  @argstab = split(/,/, $args);
  $template = "T";
  foreach $onearg (@argstab) {
    ($onearg =~ m/<(.*)>/) && ($template = $1);
    die "invalid argument : $onearg"
      if ($onearg ne "async" and $onearg ne "reply" and $onearg ne ""
          and not ($onearg =~ m/<.*>/));
  }

  $varname = "__transcript_name_".$varindex;

# call to an allocator ?
  $allocator = $template;
  $allocator = "DistAllocator<".$template.">"
    if ($fct eq "New" or $fct eq "AsyncDelete" or $fct eq "SyncDelete");

# any parameter ?
  $nbparam = ", ";
  $nbparam = "" if (($firstcars =~ m/^[ \t]*\)/) or
                    ($firstcars =~ m/^[ \t]*void[ \t]*\)/));
     
# synchronous call ? (default : RPC (reply))
  $sync = "RPC";
  $sync = "MSG" if ($args =~ m/async/);


  $input =~ s/^.*@[^:]*:[^(]*\(/dist_obj<$template> $varname($pair);\n$prev\nremote$sync($varname _THIS_IS_A_DOT_9876_ pnode, $varname _THIS_IS_A_DOT_9876_ obj, &$allocator ::$fct $nbparam/m;
}

# produce output
$input =~ s/_THIS_IS_A_DOT_9876_/./g;
print $input;

exit 0;

__END__

