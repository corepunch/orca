(StackView Name="Quote" Direction="Vertical" Spacing=16 BackgroundColor="$card-background" Padding=28
  (TextBlock Name="QuoteMark" Text="“" FontSize=48 ForegroundColor="{../Card.PrimaryColor}")
  (TextBlock Name="QuoteText" FontSize=13 ForegroundColor="$text-secondary" WordWrap="true" Text="{../Card.Body}")
  (StackView Name="QuoteAuthor" Direction="Horizontal" Spacing=12 AlignItems="Center"
    (TextBlock Name="QuoteAuthorName" FontSize=13 ForegroundColor="$text-primary" Text="{../../Card.Title}")
    (TextBlock Name="QuoteAuthorRole" FontSize=11 ForegroundColor="$text-dim" Text="{../../Card.Subtitle}")))
